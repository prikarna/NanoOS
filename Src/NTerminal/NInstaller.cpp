/*
 * File    : NInstaller.cpp
 * Remark  : NInstaller implementation.
 *
 */

#include "NInstaller.h"
#include "..\UsbData.h"

// C'tor
NInstaller::NInstaller(void):
	m_fReInstall(false),
	m_hInstThread(NULL),
	m_bUseExtPort(false),
	m_pExtPort(NULL),
	m_bPrevReadEvent(false),
	m_dwError(0)
{
	IsInstalling.Set(this, 0, &NInstaller::_GetIsInstalling);
	LastError.Set(this, 0, &NInstaller::_GetLastError);

	RtlZeroMemory(m_szAppFileName, sizeof(m_szAppFileName));
}

// D'tor
NInstaller::~NInstaller(void)
{
	if (m_Port.IsOpen)
		m_Port.Close();
}

bool NInstaller::Install(const TCHAR * szNanoOSIoName, const TCHAR *szAppFileName)
/*
	Desc.   : 
		Start NanoOS application installation in another thread.
	Params. :
		szNanoOSIoName
			Name of the NanoOS port name.
		szAppFileName
			NanoOS application file name.
	Return  : true if success otherwise false.
*/
{
	if (m_hInstThread) {
		_TriggerError(_T("Install already on progress."), ERROR_BUSY);
		return false;
	}

	if ((szNanoOSIoName == NULL) ||
		(szAppFileName == NULL))
	{
		_TriggerError(_T("Can't install: Invalid IO name and/or app. file name."), ERROR_INVALID_PARAMETER);
		return false;
	}

	if (m_fReInstall == false) {

		if (!szAppFileName) return false;

		int	iLen = lstrlen(szAppFileName);
		if ((iLen * sizeof(TCHAR)) >= (sizeof(m_szAppFileName) - 2))
		{
			_TriggerError(_T("Can't install: File name too long."), ERROR_FILENAME_EXCED_RANGE);
			return false;
		}

		RtlZeroMemory(m_szAppFileName, sizeof(m_szAppFileName));
		StringCbCopy(m_szAppFileName, sizeof(m_szAppFileName), szAppFileName);

	} else {

		m_fReInstall = false;

		if (lstrlen(m_szAppFileName) <= 0)
		{
			_TriggerError(_T("Can't reinstall: file buffer empty."), ERROR_EMPTY);
			return false;
		}
	}

	m_Port.ReadEvent = false;
	if (!m_Port.IsOpen) {
		if (!m_Port.Open(
						szNanoOSIoName, 
						921600, 
						NPort::ByteSize_8, 
						NPort::Parity_None, 
						NPort::StopBits_1, 
						NPort::DTRControl_Enable,
						NPort::RTSControl_Enable
						)) 
		{
			DWORD dwErr = m_Port.LastError;
			_TriggerError(_T("Can't open device."), dwErr);
			return false;
		}
	}

	m_bCancel = false;
	m_hInstThread = CreateThread(
								NULL,
								0,
								(LPTHREAD_START_ROUTINE) &NInstaller::_Install,
								reinterpret_cast<LPVOID>(this),
								0,
								NULL
								);
	if (!m_hInstThread) {
		_TriggerError(_T("Can't create install thread."), GetLastError());
	}

	return ((m_hInstThread) ? true : false);
}

bool NInstaller::ReInstall(const TCHAR *szNanoOSIoName)
/*
	Desc.   : Start NanoOS application re-installation in another thread.
	Params. :
		szNanoOSIoName
			Name of the NanoOS port name.
	Return  : true if success otherwise false.
*/
{
	m_fReInstall = true;

	if (lstrlen(m_szAppFileName) == 0) {
		_TriggerError(_T("Can't reinstall: No previously install application."), ERROR_EMPTY);
		m_fReInstall = false;
		return false;
	}

	return NInstaller::Install(szNanoOSIoName, m_szAppFileName);
}

bool NInstaller::Cancel()
/*
	Desc.   : Cancel current installation process.
	Params. : None.
	Return  : true if success otherwise false.
*/
{
	m_bCancel = true;

	if (!m_bUseExtPort) {
		m_Port.Close();
	}

	HANDLE	hThread;
	hThread = (HANDLE) InterlockedExchangePointer(&m_hInstThread, NULL);
	if (hThread) {
		DWORD	dwWait = WaitForSingleObject(hThread, 1000);
		if (dwWait == WAIT_TIMEOUT) {
			TerminateThread(hThread, ERROR_OPERATION_ABORTED);
		}
		CloseHandle(hThread);
	}

	if (m_bUseExtPort) {
		m_pExtPort->ReadEvent = m_bPrevReadEvent;
	}
	
	RtlZeroMemory(m_szAppFileName, sizeof(m_szAppFileName));

	return true;
}

void NInstaller::_TriggerError(const TCHAR *szErrMsg, DWORD dwError)
/*
	Desc.   : Format error message and trigger OnError when installation process fail.
	Params. :
		szErrMsg
			String error message.
		dwError
			Error code.
	Return  : None.
*/
{
	LPTSTR		pErrStr = NULL;
	DWORD		dwRes = FALSE;
	size_t		stLen = 0;

	if (dwError == 0) {
		OnError(szErrMsg);

#ifdef _DEBUG
		OutputDebugString(szErrMsg);
#endif
		return;
	}

	RtlZeroMemory(m_szErrorBuffer, sizeof(m_szErrorBuffer));
	dwRes = FormatMessage(
						FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_FROM_SYSTEM |
						FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						dwError,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						reinterpret_cast<LPTSTR>(&pErrStr),
						0,
						NULL
						);
	if (dwRes > 0) {
		stLen = lstrlen(pErrStr);
		pErrStr[stLen - 1] = _T('\0');
		pErrStr[stLen - 2] = _T('\0');
		StringCbPrintf(
					m_szErrorBuffer, 
					sizeof(m_szErrorBuffer), 
					_T("%s: %s"), 
					szErrMsg, 
					reinterpret_cast<LPTSTR>(pErrStr)
					);
		LocalFree(reinterpret_cast<HLOCAL>(pErrStr));
	} else {
		StringCbPrintf(m_szErrorBuffer, sizeof(m_szErrorBuffer), _T("%s"), szErrMsg);
	}

	m_dwError = dwError;
	OnError(&m_szErrorBuffer[0]);

	DBG_PRINTF(_T("%s\r\n"), m_szErrorBuffer);
}

bool NInstaller::_GetResponse(DWORD *pdwErr)
/*
	Desc.   : Get response from NanoOS after sending an installation command.
	Params. :
		pdwErr
			Pointer to DWORD error code that will receive error code (optional).
	Return  : true if success otherwise false.
*/
{
	bool					b = false;
	unsigned int			uiRead = 0;
	USB_INSTALL_RESPONSE	uResp;
	TCHAR					szErrCode[64];

	if (pdwErr)
		*pdwErr = ERROR_IO_DEVICE;

	RtlZeroMemory(&uResp, sizeof(USB_INSTALL_RESPONSE));
	Sleep(5);
	if (m_bUseExtPort) {
		b = m_pExtPort->Read(
							reinterpret_cast<unsigned char *>(&uResp),
							sizeof(uResp),
							uiRead
							);
		if (!b) {
			if (pdwErr) *pdwErr = m_pExtPort->LastError;
		}
	} else {
		b = m_Port.Read(
						reinterpret_cast<unsigned char *>(&uResp),
						sizeof(uResp),
						uiRead
						);
		if (!b) {
			if (pdwErr) *pdwErr = m_Port.LastError;
		}
	}

	//DBG_PRINTF(_T("%s: res.=%d, uiRead=%d, code=%d\r\n"), FUNCT_NAME_STR, b, uiRead, uResp.Code);
	
	if (!b) return false;

	if (pdwErr) *pdwErr = ERROR_GEN_FAILURE;

	b = false;
	switch (uResp.Code)
	{
	case USB_INST_RESP__SUCCESS:
		if (pdwErr)
			*pdwErr = ERROR_SUCCESS;
		b = true;
		break;

	case USB_INST_RESP__NONE:
		_TriggerError(_T("Invalid response."), ERROR_GEN_FAILURE);
		break;

	case ERR__INVALID_FLASH_PAGE_NO:
		_TriggerError(_T("Invalid page number."), ERROR_GEN_FAILURE);
		break;

	case ERR__NO_FLASH_PAGE_FOUND:
		_TriggerError(_T("No flash page found."), ERROR_GEN_FAILURE);
		break;

	case ERR__FLASH_OPERATION:
		_TriggerError(_T("Flash operation."), ERROR_GEN_FAILURE);
		break;

	case ERR__FLASH_PROGRAMMING:
		_TriggerError(_T("Flash programming."), ERROR_GEN_FAILURE);
		break;

	case ERR__FLASH_WRITE_PROTECTION:
		_TriggerError(_T("Flash write protection."), ERROR_GEN_FAILURE);
		break;

	case ERR__INVALID_FLASH_OPERATION_STATE:
		_TriggerError(_T("Invalid operation state."), ERROR_GEN_FAILURE);
		break;

	case ERR__INVALID_FLASH_NUMBER_OF_OPERATION:
		_TriggerError(_T("Invalid flash number of operation."), ERROR_GEN_FAILURE);
		break;

	case ERR__FAIL_TO_UNLOCK_FLASH:
		_TriggerError(_T("Fail to unlock."), ERROR_GEN_FAILURE);
		break;

	case ERR__FAIL_TO_LOCK_FLASH:
		_TriggerError(_T("Fail to lock."), ERROR_GEN_FAILURE);
		break;

	case ERR__APP_SIZE_TOO_BIG:
		_TriggerError(_T("Application size is too big."), ERROR_GEN_FAILURE);
		break;

	case ERR__FAIL_TO_ERASE_FLASH:
		_TriggerError(_T("Fail to erase flash."), ERROR_GEN_FAILURE);
		break;

	case ERR__FAIL_TO_PROGRAM_FLASH:
		_TriggerError(_T("Fail to program flash."), ERROR_GEN_FAILURE);
		break;

	case ERR__INVALID_DEVICE_MODE:
		_TriggerError(_T("Invalid mode."), ERROR_GEN_FAILURE);
		break;

	default:
		RtlZeroMemory(szErrCode, sizeof(szErrCode));
		StringCbPrintf(szErrCode, sizeof(szErrCode), _T("%d."), uResp.Code);
		_TriggerError(szErrCode, ERROR_GEN_FAILURE);
		DBG_PRINTF(_T("%s: Error 0x%X\r\n"), FUNCT_NAME_STR, uResp.Code);
		break;
	}

	return b;
}

DWORD CALLBACK NInstaller::_Install(LPVOID pDat)
/*
	Desc.   : 
		Installation operation thread function. Trigger OnError event when failed or OnInstalling while
		operation is on progress.
	Params. :
		pDat
			Pointer to installation data to be passed to this function.
	Return  : thread exit code.
*/
{
	NInstaller	*		pThis;
	DWORD				dwRet = 0;
	bool				bRes = false;

	pThis = reinterpret_cast<NInstaller *>(pDat);

	DWORD	dwErr = 0;
	SetLastError(0);
	HANDLE		hFile = CreateFile(
								pThis->m_szAppFileName, 
								GENERIC_READ, 
								FILE_SHARE_READ, 
								NULL, 
								OPEN_EXISTING, 
								FILE_ATTRIBUTE_NORMAL, 
								NULL
								);
	if (hFile == INVALID_HANDLE_VALUE) 
	{
		dwErr = GetLastError();
		pThis->_TriggerError(_T("Can't open application file."), dwErr);

		pThis->m_Port.Close();

		ExitThread(dwErr);
		return dwErr;
	}

	LARGE_INTEGER		liFileSize;
	BOOL				fRes = FALSE;
	USB_DATA			UsbDat;
	PUSB_INSTALL_PACKET	pPack;
	bool				bDevRes;

	RtlZeroMemory(&UsbDat, sizeof(USB_DATA));
	UsbDat.Type = USB_DATA_TYPE__INSTALL;
	pPack = &(UsbDat.u.InstallPacket);

	do {
		RtlZeroMemory(&liFileSize, sizeof(LARGE_INTEGER));
		SetLastError(0);
		fRes = GetFileSizeEx(hFile, &liFileSize);
		if (!fRes) {
			dwErr = GetLastError();
			pThis->_TriggerError(_T("Can't get application file size."), dwErr);
			break;
		}

		if (liFileSize.QuadPart == 0) {
			dwErr = ERROR_EMPTY;
			pThis->_TriggerError(_T("Can't install empty application file (file size = 0 bytes)."), dwErr);
			break;
		}

		RtlZeroMemory(pPack, sizeof(USB_INSTALL_PACKET));
		
		pPack->Index = 0xFFFF;
		pPack->Length = (unsigned short) (liFileSize.QuadPart / 1024);
		if ((liFileSize.QuadPart % 1024) > 0) 
			pPack->Length++;

		DBG_PRINTF(_T("%s: Type=%d, Index=0x%X, len.=%d\r\n"), FUNCT_NAME_STR, UsbDat.Type, pPack->Index, pPack->Length);

		bDevRes = pThis->m_Port.Write((const unsigned char *) &UsbDat, sizeof(USB_DATA));
		if (!bDevRes) {
			dwErr = pThis->m_Port.LastError;
			pThis->_TriggerError(_T("Can't write command to erase flash."), dwErr);
			break;
		}
		
		pThis->_GetResponse(&dwErr);

	} while (FALSE);

	if (dwErr != ERROR_SUCCESS) {
		dwRet = dwErr;
		goto Cleanup;
	}

	if (pThis->m_bCancel) {
		dwErr = ERROR_OPERATION_ABORTED;
		pThis->_TriggerError(_T("Operation cancelled."), dwErr);
		dwRet = dwErr;
		goto Cleanup;
	}

	int		iIndex = 0;
	int		iIter = 0;
	BOOL	fIsFinish = FALSE;
	LARGE_INTEGER	liBytesRead;
	int				iProgress = 0;
	DWORD			dwReadRes = 0;
	DWORD			dwBytesRead = 0;

	liBytesRead.QuadPart = 0;

	while (!fIsFinish)
	{
		RtlZeroMemory(pPack, sizeof(USB_INSTALL_PACKET));
		for (iIter = 0; iIter < sizeof(pPack->Data); iIter += 2)
		{
			SetLastError(0);
			fRes = ReadFile(
							hFile,
							reinterpret_cast<LPVOID>(&(pPack->Data[iIter])),
							2,
							&dwReadRes,
							NULL
							);
			dwErr = GetLastError();
			if ((dwErr == 0) && (dwReadRes == 0)) {
				fIsFinish = TRUE;
				break;
			} else if (dwReadRes > 0) {
				dwBytesRead += dwReadRes;
			} else {
				pThis->_TriggerError(_T("Can't read application file."), dwErr);
				break;
			}
		}

		if (dwErr != ERROR_SUCCESS) {
			dwRet = dwErr;
			break;
		}

		if ((iIndex == 0xFFFF) || (iIndex == 0xFFFE)) {
			dwErr = ERROR_INVALID_INDEX;
			pThis->_TriggerError(_T("Can't write application to NanoOS."), dwErr);
			dwRet = dwErr;
			break;
		}

		if (fIsFinish) {
			pPack->Index = 0xFFFE;
		} else {
			pPack->Index = iIndex;
		}
		iIndex++;
		pPack->Length = static_cast<unsigned short>(iIter);

		DBG_PRINTF(_T("%s: Packet: Index=%d, length=%d, data length=%d\r\n"), 
			FUNCT_NAME_STR, pPack->Index, pPack->Length, dwBytesRead);

		bDevRes = pThis->m_Port.Write((const unsigned char *) &UsbDat, sizeof(USB_DATA));
		if (!bDevRes) {
			dwErr = pThis->m_Port.LastError;
			pThis->_TriggerError(_T("Can't write application to NanoOS."), dwErr);
			dwRet = dwErr;
			break;
		}

		pThis->_GetResponse(&dwErr);
		if (dwErr != ERROR_SUCCESS) {
			dwRet = dwErr;
			break;
		}

		if (pThis->m_bCancel) {
			dwRet = ERROR_OPERATION_ABORTED;
			pThis->_TriggerError(_T("Operation cancelled."), dwRet);
			break;
		} else {
			liBytesRead.QuadPart = dwBytesRead;
			iProgress = (int) ((liBytesRead.QuadPart * 100) / liFileSize.QuadPart);
			pThis->OnInstalling(iProgress);
		}
	}

Cleanup:
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	pThis->m_Port.Close();

	HANDLE	hTh = (HANDLE) InterlockedExchangePointer(&(pThis->m_hInstThread), NULL);
	if (hTh) {
		CloseHandle(hTh);
	}

	DBG_PRINTF(_T("%s: terminated gracefully.\r\n"), FUNCT_NAME_STR);

	ExitThread(dwRet);
	return dwRet;
}

bool NInstaller::_EraseFlash(HANDLE hAppFile, PLARGE_INTEGER pliFileSize, DWORD * pdwErr)
/*
	Desc.   : Erase flash operation.
	Params. :
		hAppFile
			Handle to opened NanoOS application file.
		pliFileSize
			NanoOS application file size.
		pdwErr
			Pointer to DWORD variable that will receive the error code.
	Return  : true if success otherwise false.
*/
{
	LARGE_INTEGER	liFileSize = {0};
	BOOL			fRes = FALSE;
	DWORD			dwErr = ERROR_SUCCESS;
	USB_DATA		UsbDat = {0};
	bool			bRes = false;

	if (!pliFileSize) return false;

	fRes = GetFileSizeEx(hAppFile, &liFileSize);
	if (!fRes) {
		dwErr = GetLastError();
		if (pdwErr) *pdwErr = dwErr;
		_TriggerError(_T("Can't get application file size"), dwErr);
		return false;
	}

	if (liFileSize.QuadPart == 0) {
		dwErr = ERROR_BAD_LENGTH;
		if (pdwErr) *pdwErr = dwErr;
		_TriggerError(_T("Invalid application file size"), dwErr);
		return false;
	}

	UsbDat.Type = USB_DATA_TYPE__INSTALL;
	UsbDat.u.InstallPacket.Index = 0xFFFF;
	UsbDat.u.InstallPacket.Length = static_cast<unsigned short>((liFileSize.QuadPart / 1024));
	if ((liFileSize.QuadPart % 1024) > 0) 
		UsbDat.u.InstallPacket.Length++;

	DBG_PRINTF(_T("%s: Index=0x%X, Length=%d\r\n"), 
		FUNCT_NAME_STR, UsbDat.u.InstallPacket.Index, UsbDat.u.InstallPacket.Length);

	if (m_bUseExtPort) {
		bRes = m_pExtPort->Write(reinterpret_cast<const unsigned char *>(&UsbDat), sizeof(USB_DATA));
		if (!bRes) dwErr = m_pExtPort->LastError;
	} else {
		bRes = m_Port.Write(reinterpret_cast<const unsigned char *>(&UsbDat), sizeof(USB_DATA));
		if (!bRes) dwErr = m_Port.LastError;
	}
	if (!bRes) {
		if (pdwErr) *pdwErr = dwErr;
		_TriggerError(_T("Can't write port."), dwErr);
		return false;
	}

	bRes = _GetResponse(&dwErr);
	if (!bRes) {
		if (pdwErr) *pdwErr = dwErr;
		return false;
	}

	*pliFileSize = liFileSize;
	if (pdwErr) *pdwErr = dwErr;
	
	return true;
}

bool NInstaller::_ProgramFlash(HANDLE hAppFile, LARGE_INTEGER liFileSize, DWORD * pdwErr)
/*
	Desc.   : Program or write to flash operation.
	Params. :
		hAppFile
			Handle to opened NanoOS application file.
		pliFileSize
			NanoOS application file size.
		pdwErr
			Pointer to DWORD variable that will receive error code.
	Return  : true if success otherwise false.
*/
{
	int					iIndex = 0;
	int					iIter = 0;
	BOOL				fIsFinish = FALSE;
	BOOL				fRes = FALSE;
	LARGE_INTEGER		liBytesRead;
	int					iProgress = 0;
	DWORD				dwReadRes = 0;
	DWORD				dwBytesRead = 0;
	DWORD				dwErr = ERROR_SUCCESS;
	USB_DATA			UsbDat = {0};
	PUSB_INSTALL_PACKET	pPack = NULL;
	bool				bRes = false;

	UsbDat.Type = USB_DATA_TYPE__INSTALL;
	pPack = (PUSB_INSTALL_PACKET) &(UsbDat.u.InstallPacket);

	liBytesRead.QuadPart = 0;

	while (!fIsFinish)
	{
		RtlZeroMemory(pPack, sizeof(USB_INSTALL_PACKET));
		for (iIter = 0; iIter < sizeof(pPack->Data); iIter += 2)
		{
			SetLastError(0);
			fRes = ReadFile(
							hAppFile,
							reinterpret_cast<LPVOID>(&(pPack->Data[iIter])),
							2,
							&dwReadRes,
							NULL
							);
			dwErr = GetLastError();
			if ((dwErr == ERROR_SUCCESS) && (dwReadRes == 0)) {
				fIsFinish = TRUE;
				break;
			} else if (dwReadRes > 0) {
				dwBytesRead += dwReadRes;
				if (dwBytesRead == (DWORD) liFileSize.QuadPart) {
					fIsFinish = TRUE;
				}
			} else {
				_TriggerError(_T("Can't read application file."), dwErr);
				break;
			}
		}

		if (dwErr != ERROR_SUCCESS) {
			if (pdwErr) *pdwErr = dwErr;
			break;
		}

		if ((fIsFinish) && (iIter == 0)) break;

		if ((iIndex == 0xFFFF) || (iIndex == 0xFFFE))
		{
			dwErr = ERROR_INVALID_INDEX;
			if (pdwErr) *pdwErr = dwErr;
			_TriggerError(_T("Can't write application file to NanoOS."), dwErr);
			break;
		}

		if (fIsFinish) {
			pPack->Index = 0xFFFE;
		} else {
			pPack->Index = static_cast<unsigned short>(iIndex);
		}
		iIndex++;
		pPack->Length = static_cast<unsigned short>(iIter);

		DBG_PRINTF(_T("%s: Packet: Index=%d, length=%d, data length=%d\r\n"), 
			FUNCT_NAME_STR, pPack->Index, pPack->Length, dwBytesRead);

		if (m_bUseExtPort) {
			bRes = m_pExtPort->Write(reinterpret_cast<const unsigned char *>(&UsbDat), sizeof(USB_DATA));
			if (!bRes) dwErr = m_pExtPort->LastError;
		} else {
			bRes = m_Port.Write(reinterpret_cast<const unsigned char *>(&UsbDat), sizeof(USB_DATA));
			if (!bRes) dwErr = m_Port.LastError;
		}
		if (!bRes) {
			if (pdwErr) *pdwErr = dwErr;
			_TriggerError(_T("Can't write application file to NanoOS."), dwErr);
			break;
		}

		bRes = _GetResponse(&dwErr);
		if (dwErr != ERROR_SUCCESS) {
			if (pdwErr) *pdwErr = dwErr;
			break;
		}

		if (m_bCancel) {
			dwErr = ERROR_OPERATION_ABORTED;
			if (pdwErr) *pdwErr = dwErr;
			_TriggerError(_T("Operation cancelled."), dwErr);
			break;
		} else {
			liBytesRead.QuadPart = dwBytesRead;
			iProgress = (int) ((liBytesRead.QuadPart * 100) / liFileSize.QuadPart);
			OnInstalling(iProgress);
		}
	}

	if (pdwErr) *pdwErr = dwErr;

	return ((dwErr == ERROR_SUCCESS) ? true : false);
}

DWORD CALLBACK NInstaller::_Install2(LPVOID pDat)
/*
	Desc.   : 
		Installation operation thread function version 2. Trigger OnError event when failed and OnInstalling
		event while operation is on progress.
	Params. :
		pDat
			Pointer to installation data to be passed to this function.
	Return  : thread exit code.
*/
{
	NInstaller	*		pThis;
	DWORD				dwRet = 0;
	bool				bRes = false;
	LARGE_INTEGER		liFileSize = {0};

	pThis = reinterpret_cast<NInstaller *>(pDat);

	SetLastError(0);
	HANDLE		hFile = CreateFile(
								pThis->m_szAppFileName, 
								GENERIC_READ, 
								FILE_SHARE_READ, 
								NULL, 
								OPEN_EXISTING, 
								FILE_ATTRIBUTE_NORMAL, 
								NULL
								);
	if (hFile == INVALID_HANDLE_VALUE) 
	{
		dwRet = GetLastError();
		pThis->_TriggerError(_T("Can't open application file."), dwRet);

		DBG_PRINTF(_T("%s: Error open app file name (%d)\r\n"), FUNCT_NAME_STR, dwRet);

		goto Cleanup;
	}

	if (pThis->m_bUseExtPort) {
		pThis->m_pExtPort->ReadEvent = false;
	} else {
		pThis->m_Port.ReadEvent = false;
	}

	bRes = pThis->_EraseFlash(hFile, &liFileSize, &dwRet);
	if (!bRes) goto Cleanup;

	bRes = pThis->_ProgramFlash(hFile, liFileSize, &dwRet);

Cleanup:
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	if (pThis->m_bUseExtPort) {
		pThis->m_pExtPort->ReadEvent = pThis->m_bPrevReadEvent;;
	} else {
		pThis->m_Port.ReadEvent = pThis->m_bPrevReadEvent;
	}

	pThis->m_bUseExtPort = false;

	HANDLE	hTh = (HANDLE) InterlockedExchangePointer(&(pThis->m_hInstThread), NULL);
	if (hTh) {
		CloseHandle(hTh);
	}

	DBG_PRINTF(_T("%s: terminated gracefully.\r\n"), FUNCT_NAME_STR);

	ExitThread(dwRet);
	return dwRet;
}

bool NInstaller::Install(NPort * pPort, const TCHAR *szAppFileName)
/*
	Desc.   : Installation operation thread function (overloaded).
	Params. :
		pPort
			Pointer to existing instance of NPort object.
		szAppFileName
			String NanoOS application file.
	Return  : true if success otherwise false.
*/
{
	DWORD	dwErr = 0;

	if (m_hInstThread) {
		_TriggerError(_T("Install in progress."), ERROR_BUSY);
		return false;
	}

	if (m_fReInstall) {
		m_fReInstall = false;
		if ((lstrlen(m_szAppFileName) == 0) || (m_pExtPort == NULL)) {
			_TriggerError(_T("No previously application program being installed."), ERROR_INVALID_PARAMETER);
			return false;
		}
	} else {
		if (!szAppFileName) {
			_TriggerError(_T("No application file name specified."), ERROR_EMPTY);
			return false;
		}
		if (!pPort) {
			_TriggerError(_T("No external NPort specified."), ERROR_INVALID_PARAMETER);
			return false;
		}
		if (lstrlen(szAppFileName) >= ((sizeof(m_szAppFileName) / sizeof(TCHAR)) - sizeof(TCHAR)))
		{
			_TriggerError(_T("Application file name is too long."), 0);
			return false;
		}
		RtlZeroMemory(&m_szAppFileName, sizeof(m_szAppFileName));
		StringCbCopy(m_szAppFileName, sizeof(m_szAppFileName), szAppFileName);

		m_pExtPort = pPort;
	}

	if (!m_pExtPort->IsOpen) {
		_TriggerError(_T("NPort is not opened."), ERROR_INVALID_STATE);
		return false;
	}

	m_bPrevReadEvent = m_pExtPort->ReadEvent;

	m_bUseExtPort = true;
	m_bCancel = false;
	m_hInstThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &NInstaller::_Install2, (LPVOID) this, 0, NULL);
	if (!m_hInstThread) {
		dwErr = GetLastError();
		_TriggerError(_T("Can't create installl thread."), dwErr);

		m_pExtPort->ReadEvent = m_bPrevReadEvent;
		
		m_pExtPort = NULL;
		m_bUseExtPort = false;
		return false;
	}

	return true;
}

bool NInstaller::ReInstall()
/*
	Desc.   : Re-installation operation (overloaded).
	Params. : None.
	Return  : true if success otherwise false.
*/
{
	m_fReInstall = true;

	return NInstaller::Install(reinterpret_cast<NPort *>(NULL), reinterpret_cast<const TCHAR *>(NULL));
}

bool NInstaller::_GetIsInstalling()
/*
	Desc.   : Get current installation status.
	Params. : None.
	Return  : true if still installing otherwise false.
*/
{
	bool	bRes = false;

	if (!m_hInstThread) return false;

	DWORD	dwExit = 0;
	BOOL	fRes = FALSE;

	__try {
		fRes = GetExitCodeThread(m_hInstThread, &dwExit);
		if (fRes) {
			if (dwExit == STILL_ACTIVE) {
				bRes = true;
			}
		}
	}
	__finally
	{
		// Do nothing.
	}
	return bRes;
}

unsigned long NInstaller::_GetLastError()
{
	return m_dwError;
}
