/*
 * File    : NPort.cpp
 * Remark  : NPort implementation.
 *
 */

#include "NPort.h"

/* List of instances of NPort class */
NPort *	NPort::m_PortList[MAX_NPORT] = {0};

/* C'tor */
NPort::NPort(void): 
	m_hDev(INVALID_HANDLE_VALUE),
	m_hReadThread(NULL),
	m_bStopRead(false),
	ReadDataSize(1024),
	m_uiInQueueSize(4096),
	m_uiOutQueueSize(4096),
	m_uiReadIntervalTimeOut(10),
	m_fNullStripping(false),
	SuppressError(false),
	m_hDetWnd(NULL),
	m_fEnableAutoDet(false),
	m_hNotif(NULL),
	m_hDetTh(NULL),
	m_szDetWndClassName(_T("DkDetWindClass")),
	m_bReadEvent(true),
	m_hRdEvtMem(NULL),
	m_pRdMem(NULL),
	m_fFormatedReadData(false),
	m_dwLastError(0)
{
	RtlZeroMemory(&m_WrOvr, sizeof(OVERLAPPED));
	RtlZeroMemory(&m_RdOvr, sizeof(OVERLAPPED));
	RtlZeroMemory(&m_szDevName, sizeof(m_szDevName));

	InitializeCriticalSection(&m_CritSec);

	Handle.Set(this, 0, &NPort::_GetHandle);
	InputQueueSize.Set(this, &NPort::_SetInputQueueSize, &NPort::_GetInputQueueSize);
	OutputQueueSize.Set(this, &NPort::_SetOutputQueueSize, &NPort::_GetOutputQueueSize);
	TimeOut.Set(this, &NPort::_SetTimeOut, &NPort::_GetTimeOut);
	IsOpen.Set(this, 0, &NPort::_GetIsOpen);
	NullStripping.Set(this, &NPort::_SetNullStripping, &NPort::_GetNullStripping);
	DeviceName.Set(this, 0, &NPort::_GetDeviceName);
	ReadEvent.Set(this, &NPort::_SetReadEvent, &NPort::_GetReadEvent);
	FormatedReadData.Set(this, &NPort::_SetFormatReadData, &NPort::_GetFormatReadData);
	LastError.Set(this, 0, &NPort::_GetLastError);

	int i;
	for (i = 0; i < MAX_NPORT; i++) {
		if (m_PortList[i] == NULL) {
			m_PortList[i] = this;
			break;
		}
	}

	if (i == MAX_NPORT)
		throw "Number of object exeeded (Max. NPort = 4)!";
}

/* D'tor */
NPort::~NPort(void)
{
	Close();

	DeleteCriticalSection(&m_CritSec);

	for (int i = 0; i < MAX_NPORT; i++) {
		if (m_PortList[i] == this) {
			m_PortList[i] = NULL;
			break;
		}
	}
}

bool NPort::Open(
				 const TCHAR *szComName,
				 unsigned int uiBaudRate,
				 NPort::ByteSize eByteSize,
				 NPort::Parity eParity,
				 NPort::StopBits eStopBits,
				 NPort::DTRControl eDTRControl,
				 NPort::RTSControl eRTSControl
				 )
 /*
	Desc.   : Open a Serial (COM) port.
	Params. :
		szComName
			Serial (COM) port name.
		uiBaudRate
			Baudrate of serial port.
		eByteSize
			Byte size of serial port, one of NPort::ByteSize enumeration.
		eParity
			Parity type, one of NPort::Parity enumeration.
		eStopBits
			Stop bits, one of NPort::StopBits enumeration.
		eDTRControl
			DTR control, one of NPort::DTRControl enumeration.
		eRTSControl
			RTS control, one of NPort::RTSControl enumeration.
	Return  : true if success otherwise false.
*/
{
	if (m_hDev != INVALID_HANDLE_VALUE) return true;

	BOOL			fRes = FALSE;
	DCB				dcb;
	COMMTIMEOUTS	cto;

	m_dwLastError = ERROR_SUCCESS;
	do {
		if (ReadDataSize < 1) {
			m_dwLastError = ERROR_BAD_LENGTH;
			_HandleError(_T("Invalid read data size (min.=1)"), 0);
			break;
		}

		m_hDev = CreateFile(
						szComName, 
						GENERIC_ALL, 
						0, 
						NULL, 
						OPEN_EXISTING, 
						FILE_FLAG_OVERLAPPED, 
						NULL
						);
		if (m_hDev == INVALID_HANDLE_VALUE) {
			m_dwLastError = GetLastError();
			_FormatString(_T("Can't open %s"), szComName);
			_HandleError(m_szGenStrBuff, m_dwLastError);
			break;
		}

		if (!m_fEnableAutoDet) {
			RtlZeroMemory(&m_szDevName, sizeof(m_szDevName));
			StringCbCopy(m_szDevName, sizeof(m_szDevName), szComName);
		}

		m_WrOvr.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!m_WrOvr.hEvent) {
			m_dwLastError = GetLastError();
			_HandleError(_T("Can't create event for overlapped operation"), m_dwLastError);
			break;
		}

		fRes = SetupComm(m_hDev, m_uiInQueueSize, m_uiOutQueueSize);
		if (!fRes) {
			m_dwLastError = GetLastError();
			_FormatString(_T("Can't set queue size on %s"), szComName);
			_HandleError(m_szGenStrBuff, m_dwLastError);
			break;
		}

		RtlZeroMemory((PVOID) &dcb, sizeof(DCB));
		dcb.DCBlength = sizeof(DCB);
		fRes = GetCommState(m_hDev, &dcb);
		if (!fRes) {
			m_dwLastError = GetLastError();
			_FormatString(_T("Can't get com state on %s"), szComName);
			_HandleError(m_szGenStrBuff, m_dwLastError);
			break;
		}

		dcb.ByteSize = static_cast<BYTE>(eByteSize);
		dcb.BaudRate = static_cast<DWORD>(uiBaudRate);
		dcb.Parity = static_cast<BYTE>(eParity);
		dcb.StopBits = static_cast<BYTE>(eStopBits);
		dcb.fDtrControl = static_cast<BYTE>(eDTRControl);
		dcb.fRtsControl = static_cast<BYTE>(eRTSControl);
		dcb.fNull = ((m_fNullStripping) ? 1 : 0);
		dcb.fBinary = TRUE;
		fRes = SetCommState(m_hDev, &dcb);
		if (!fRes) {
			m_dwLastError = GetLastError();
			_FormatString(_T("Can't set state on %s"), szComName);
			_HandleError(m_szGenStrBuff, m_dwLastError);
			break;
		}

		RtlZeroMemory((PVOID) &cto, sizeof(COMMTIMEOUTS));
		cto.ReadIntervalTimeout = m_uiReadIntervalTimeOut;
		fRes = SetCommTimeouts(m_hDev, &cto);
		if (!fRes) {
			m_dwLastError = GetLastError();
			_FormatString(_T("Can't set time out on %s"), szComName);
			_HandleError(m_szGenStrBuff, m_dwLastError);
			break;
		}

		m_RdOvr.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!m_RdOvr.hEvent) {
			m_dwLastError = GetLastError();
			_HandleError(_T("Can't create event for overlapped operation"), m_dwLastError);
			break;
		}

		m_hRdEvtMem = GlobalAlloc(GPTR, (ReadDataSize * 2));
		if (!m_hRdEvtMem) {
			m_dwLastError = GetLastError();
			_HandleError(_T("Can't allocate global memory"), m_dwLastError);
			break;
		}
		m_pRdMem = GlobalLock(m_hRdEvtMem);
		if (!m_pRdMem) {
			m_dwLastError = GetLastError();
			_HandleError(_T("Can't locl global memory"), m_dwLastError);
			break;
		}

		if (m_bReadEvent)
		{
			m_bStopRead = false;
			m_hReadThread = CreateThread(
									NULL,
									0,
									reinterpret_cast<LPTHREAD_START_ROUTINE>(&NPort::_Read),
									reinterpret_cast<LPVOID>(this),
									0,
									NULL
									);
			if (!m_hReadThread) {
				m_dwLastError = GetLastError();
				_FormatString(_T("Can't create thread to read %s"), szComName);
				_HandleError(m_szGenStrBuff, m_dwLastError);
				break;
			}
		}

		m_dwLastError = ERROR_SUCCESS;
	} while (FALSE);

	if (m_dwLastError != ERROR_SUCCESS) {

		if (m_hDev != INVALID_HANDLE_VALUE) {
			CloseHandle(m_hDev);
			m_hDev = INVALID_HANDLE_VALUE;
		}
		if (m_hReadThread) {
			TerminateThread(m_hReadThread, m_dwLastError);
			CloseHandle(m_hReadThread);
			m_hReadThread = NULL;
		}
		if (m_WrOvr.hEvent) {
			CloseHandle(m_WrOvr.hEvent);
			m_WrOvr.hEvent = NULL;
		}
		if (m_RdOvr.hEvent) {
			CloseHandle(m_RdOvr.hEvent);
			m_RdOvr.hEvent = NULL;
		}
		if (m_pRdMem) {
			GlobalUnlock(m_pRdMem);
			m_pRdMem = NULL;
		}
		if (m_hRdEvtMem) {
			GlobalFree(m_hRdEvtMem);
			m_hRdEvtMem = NULL;
		}
		RtlZeroMemory(&m_szDevName, sizeof(m_szDevName));
	}

	return ((m_dwLastError == ERROR_SUCCESS) ? true : false);
}

bool NPort::Close()
/*
	Desc.   : Close Serial (COM) port if opened.
	Params. : None.
	Return  : true if success otherwise false.
*/
{
	BOOL	fRes = FALSE;
	DWORD	dwWait = 0;
	HANDLE	hThread = 0;

	if (m_hDev == INVALID_HANDLE_VALUE) return true;
	
	m_bStopRead = true;
	__try {
		fRes = CloseHandle(m_hDev);
	}
	__finally
	{
		// Do nothing;
	}

	hThread = (HANDLE) InterlockedExchangePointer((LPVOID) &m_hReadThread, NULL);
	if (hThread) {
		dwWait = WaitForSingleObject(hThread, 800);
		if (dwWait == WAIT_TIMEOUT) {
			__try {
				fRes = TerminateThread(hThread, dwWait);
				fRes = CloseHandle(hThread);
			}
			__finally
			{
				// Do nothing
			}
		}
	}

	m_hDev = INVALID_HANDLE_VALUE;

	if (m_WrOvr.hEvent) {
		fRes = CloseHandle(m_WrOvr.hEvent);
		m_WrOvr.hEvent = NULL;
	}
	if (m_RdOvr.hEvent) {
		fRes = CloseHandle(m_RdOvr.hEvent);
		m_RdOvr.hEvent = NULL;
	}
	if (m_pRdMem) {
		GlobalUnlock(m_hRdEvtMem);
		m_pRdMem = NULL;
	}
	if (m_hRdEvtMem) {
		GlobalFree(m_hRdEvtMem);
		m_hRdEvtMem = NULL;
	}

	if (!m_fEnableAutoDet)
		RtlZeroMemory(m_szDevName, sizeof(m_szDevName));

	return ((fRes) ? true : false);
}

DWORD CALLBACK NPort::_Read(LPVOID pDat)
/*
	Desc.   : 
		Read serial port thread, use to read serial port in another thread continously and trigger OnFormatedReadData 
		or OnDataReceived event (depend on FormatedReadData property) when receive a data from serial port.
	Params. : 
		pDat
			Pointer to data to passed to this function.
	Return  : thread exit code.
*/
{
	NPort*		pThis = reinterpret_cast<NPort *>(pDat);
	DWORD		dwRead;
	DWORD		dwReqRead;
	BOOL		fRes = FALSE;
	DWORD		dwErr = 0;
	char		*pReadDat;
	OVERLAPPED	Ovr = {0};
	NPort::ReadDataFormat		FmtDat;

	if (!pThis) {
		ExitThread(ERROR_INVALID_DATA);
		return ERROR_INVALID_DATA;
	}

	Ovr.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!Ovr.hEvent) {
		dwErr = GetLastError();
		pThis->m_dwLastError = dwErr;
		pThis->_HandleError(_T("Can't create event"), dwErr);
		goto EndRead;
	}

	dwReqRead = pThis->ReadDataSize;

	pReadDat = reinterpret_cast<char *>(pThis->m_pRdMem);
	FmtDat.RawData = pThis->m_pRdMem;

	while (pThis->m_bStopRead == false) {
		dwRead = 0;
		fRes = ReadFile(
					pThis->m_hDev, 
					pThis->m_pRdMem,
					dwReqRead,
					&dwRead,
					&Ovr
					);
		dwErr = GetLastError();
		if (dwErr == ERROR_IO_PENDING) {
			fRes = GetOverlappedResult(
									pThis->m_hDev,
									&Ovr,
									&dwRead,
									TRUE
									);
		}

		if (!fRes) {
			dwErr = GetLastError();
			pThis->m_dwLastError = dwErr;
			if (dwErr != ERROR_OPERATION_ABORTED) {
				pThis->_HandleError(_T("Error while read"), dwErr);
			}
			break;
		}

		if (pThis->m_bStopRead) break;

		if (dwRead > 0) {
			EnterCriticalSection(&(pThis->m_CritSec));
			if (pThis->m_fFormatedReadData) {
				FmtDat.DataLength = static_cast<unsigned int>(dwRead);
				pThis->OnFormatedReadData(&FmtDat);
			} else {
				pThis->OnDataReceived(pReadDat);
			}
			LeaveCriticalSection(&(pThis->m_CritSec));
		} else {
			DBG_PRINTF(_T("%s: Warning: zero data len.\r\n"), FUNCT_NAME_STR);
		}

		RtlZeroMemory(pThis->m_pRdMem, (pThis->ReadDataSize * 2));
	}

	if (Ovr.hEvent)
		CloseHandle(Ovr.hEvent);

EndRead:
	DBG_PRINTF(_T("%s: Terminated gracefully.\r\n"), FUNCT_NAME_STR);

	ExitThread(dwErr);
	return dwErr;
}

void NPort::_HandleError(const TCHAR *szErrorMessage, DWORD dwError)
/*
	Desc.   : 
		Format error string and trigger OnError. This error message can be suppressed depend on SuppressError 
		variable (true or false).
	Params. : 
		szErrorMessage
			String error message.
		dwError
			Error code to formatted.
	Return  : None.
*/
{
	LPTSTR		pErrStr = NULL;
	DWORD		dwRes = FALSE;
	size_t		stLen = 0;

#ifdef _DEBUG

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
					szErrorMessage, 
					reinterpret_cast<LPTSTR>(pErrStr)
					);
		LocalFree(reinterpret_cast<HLOCAL>(pErrStr));
	} else {
		StringCbPrintf(m_szErrorBuffer, sizeof(m_szErrorBuffer), _T("%s"), szErrorMessage);
	}

	if (!NPort::SuppressError) {
		OnError(&m_szErrorBuffer[0]);
	}

	DbgPrintf(_T("%s\r\n"), m_szErrorBuffer);

#else

	if (NPort::SuppressError) {
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
					_T("%s: %s."), 
					szErrorMessage, 
					reinterpret_cast<LPTSTR>(pErrStr)
					);
		LocalFree(reinterpret_cast<HLOCAL>(pErrStr));
	} else {
		StringCbPrintf(m_szErrorBuffer, sizeof(m_szErrorBuffer), _T("%s."), szErrorMessage);
	}

	OnError(&m_szErrorBuffer[0]);

#endif
}

bool NPort::Write(unsigned char ucDat)
/*
	Desc.   : Write a byte to serial port and trigger OnError when failed.
	Params. : 
		ucDat
			Byte data to write to serial port.
	Return  : true if success or otherwise false.
*/
{
	if (m_hDev == INVALID_HANDLE_VALUE) return false;

	BOOL	fRes = TRUE;
	DWORD	dwRet = 0;

	m_dwLastError = ERROR_SUCCESS;
	fRes = WriteFile(
					m_hDev, 
					reinterpret_cast<LPCVOID>(&ucDat), 
					1, 
					&dwRet, 
					&m_WrOvr
					);
	m_dwLastError = GetLastError();
	if (m_dwLastError == ERROR_IO_PENDING) {
		fRes = GetOverlappedResult(m_hDev, &m_WrOvr, &dwRet, TRUE);
	}
	if (!fRes) {
		m_dwLastError = GetLastError();
		_HandleError(_T("Write error"), m_dwLastError);
	}

	return (fRes) ? true : false;
}

bool NPort::Write(const unsigned char *pDat, unsigned int uiDataSize)
/*
	Desc.   : Write byte stream with specified length to serial port and trigger OnError when failed.
	Params. : 
		pDat
			Pointer to byte stream data.
		uiDataSize
			Byte stream data size of length.
	Return  : true if success or otherwise false.
*/
{
	if (m_hDev == INVALID_HANDLE_VALUE) return false;

	BOOL	fRes = TRUE;
	DWORD	dwRet = 0;

	m_dwLastError = ERROR_SUCCESS;
	fRes = WriteFile(
					m_hDev, 
					reinterpret_cast<LPCVOID>(pDat), 
					static_cast<DWORD>(uiDataSize), 
					&dwRet, 
					&m_WrOvr
					);
	m_dwLastError = GetLastError();
	if (m_dwLastError == ERROR_IO_PENDING) {
		fRes = GetOverlappedResult(m_hDev, &m_WrOvr, &dwRet, TRUE);
	}
	if (!fRes) {
		m_dwLastError = GetLastError();
		_HandleError(_T("Write error"), m_dwLastError);
	}

	return (fRes) ? true : false;
}

void NPort::EnumComm()
/*
	Desc.   : 
		Enumerate serial or COM port available on the system and trigger OnEnumCOM when found 
		a serial or COM port.
	Params. : None.
	Return  : None.
*/
{
	TCHAR	szComName[128];
	TCHAR	szDosDev[512];
	DWORD	dwRes;

	for (int i = 0; i < 100; i++) {
		RtlZeroMemory(szComName, sizeof(szComName));
		StringCbPrintf(szComName, sizeof(szComName), _T("COM%d"), i);
		RtlZeroMemory(szDosDev, sizeof(szDosDev));
		dwRes = QueryDosDevice(szComName, szDosDev, sizeof(szDosDev)/sizeof(TCHAR));
		if (dwRes > 0) {
			DBG_PRINTF(_T("%s: Comm name: %s, DOS dev. name: %s\r\n"), FUNCT_NAME_STR, szComName, szDosDev);
			OnEnumCOM(szComName);
		}
	}
}

void NPort::_FormatString(const TCHAR *szFormat, ...)
/*
	Desc.   : Format a string to general string buffer.
	Params. : 
		szFormat
			String format, follow the StringCbVPrintf() format.
	Return  : None.
*/
{
	va_list			al;

	va_start(al, szFormat);
	StringCbVPrintf(m_szGenStrBuff, sizeof(m_szGenStrBuff), szFormat, al);
	va_end(al);
}

bool NPort::Read(unsigned char *pBuffer, unsigned int uiBufferSize, unsigned int &uiRead)
/*
	Desc.   : Read serial port for specified buffer and length and trigger OnError if failed.
	Params. : 
		pBuffer
			Pointer to byte buffer to receive data when read operation success.
		uiBufferSize
			Byte buffer size or length in bytes.
		uiRead
			Reference to unsigned int variable that will receive number of byte that has
			been successfully read.
	Return  : true if success or otherwise false.
*/
{
	if (m_hDev == INVALID_HANDLE_VALUE) return false;
	if ((!pBuffer) || (uiBufferSize == 0)) return false;

	BOOL	fRes = TRUE;
	DWORD	dwRet = 0;

	m_dwLastError = ERROR_SUCCESS;
	ReadFile(
			m_hDev, 
			reinterpret_cast<LPVOID>(pBuffer), 
			static_cast<DWORD>(uiBufferSize), 
			&dwRet, 
			&m_RdOvr
			);
	m_dwLastError = GetLastError();
	if (m_dwLastError == ERROR_IO_PENDING) {
		fRes = GetOverlappedResult(m_hDev, &m_RdOvr, &dwRet, TRUE);
	}
	if (!fRes) {
		m_dwLastError = GetLastError();
		_HandleError(_T("Read error"), m_dwLastError);
	} else {
		uiRead = static_cast<unsigned int>(dwRet);
	}

	return (fRes) ? true : false;
}

bool NPort::Purge(enum NPort::PurgeOption Option)
{
	DWORD	dwFlags = 0;

	if (m_hDev == INVALID_HANDLE_VALUE) return false;

	switch (Option)
	{
	case PurgeOption_RxAbort:
		dwFlags = PURGE_RXABORT;
		break;

	case PurgeOption_RxClear:
		dwFlags = PURGE_RXCLEAR;
		break;

	case PurgeOption_TxAbort:
		dwFlags = PURGE_TXABORT;
		break;

	case PurgeOption_TxClear:
		dwFlags = PURGE_TXCLEAR;
		break;

	case PurgeOption_RxTxAbort:
		dwFlags = PURGE_RXABORT | PURGE_TXABORT;
		break;

	case PurgeOption_RxTxClear:
		dwFlags = PURGE_RXCLEAR | PURGE_TXCLEAR;
		break;

	default:
		dwFlags = PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR;
		break;
	}

	BOOL	fRes = PurgeComm(m_hDev, dwFlags);
	return ((fRes) ? true : false);
}

bool NPort::Read(unsigned char * pBuffer)
/*
	Desc.   : Read a byte data from serial port.
	Params. : 
		pBuffer
			Pointer to byte buffer to receive byte data.
	Return  : true if success or otherwise false.
*/
{
	if (m_hDev == INVALID_HANDLE_VALUE) return false;
	if (!pBuffer) return false;

	BOOL	fRes = TRUE;
	DWORD	dwRet = 0;

	m_dwLastError = ERROR_SUCCESS;
	ReadFile(
			m_hDev, 
			reinterpret_cast<LPVOID>(pBuffer), 
			1, 
			&dwRet, 
			&m_RdOvr
			);
	m_dwLastError = GetLastError();
	if (m_dwLastError == ERROR_IO_PENDING) {
		fRes = GetOverlappedResult(m_hDev, &m_RdOvr, &dwRet, TRUE);
	}
	if (!fRes) {
		m_dwLastError = GetLastError();
		_HandleError(_T("Read error"), m_dwLastError);
	}

	return (fRes) ? true : false;
}

LRESULT CALLBACK NPort::_DetectProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm)
/*
	Desc.   : 
		A window procedure (A.K.A WNDPROC) of detecting message only window. This function will forward 
		to instances of NPort class via NPort::_Proc function.
	Params. : (Same as WNDPROC in documentation)
	Return  : (Same as WNDPROC in documentation).
*/
{
	bool	bRes = false;
	NPort*	pPort;

	for (int i = 0; i < MAX_NPORT; i++) {
		pPort = NPort::m_PortList[i];
		if (!pPort) continue;

		if (pPort->m_hDetWnd == hWnd) {
			bRes = pPort->_HandleDetectProc(uMsg, wParm, lParm);
			break;
		}
	}

	if (uMsg == WM_DEVICECHANGE) {
		return static_cast<LRESULT>(TRUE);
	}

	if (!bRes) 
		return DefWindowProc(hWnd, uMsg, wParm, lParm);

	return static_cast<LRESULT>(FALSE);
}

bool NPort::_HandleDetectProc(UINT uMsg, WPARAM wParm, LPARAM lParm)
/*
	Desc.   : 
		Handle _DetectProc window procedure for current instance. This is the part of auto 
		detection mechanism.
	Params. : 
		uMsg 
			(Same as WNDPROC in documentation)
		wParm
			(Same as WNDPROC)
		lParm
			(Same as WNDPROC)
	Return  : true if handled or otherwise false.
*/
{
	bool							fRes = false;
	PDEV_BROADCAST_HDR				pHdr = NULL;
	PDEV_BROADCAST_DEVICEINTERFACE	pDevIface = NULL;
	bool							fDatRes = false;

	switch (uMsg)
	{
	case WM_CLOSE:
		if (m_hNotif) {
			UnregisterDeviceNotification(m_hNotif);
			m_hNotif = NULL;
		}
		DestroyWindow(m_hDetWnd);
		fRes = true;
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		fRes = true;
		break;

	case WM_DEVICECHANGE:
		pHdr = reinterpret_cast<PDEV_BROADCAST_HDR>(lParm);
		if (pHdr) 
		{
			if (pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) 
			{
				pDevIface = (PDEV_BROADCAST_DEVICEINTERFACE) pHdr;
			}
		}
		switch (wParm)
		{
		case DBT_DEVICEARRIVAL:
			if (pDevIface) 
			{
				if (lstrcmpi(m_szDevName, pDevIface->dbcc_name) == 0) 
				{
					DBG_PRINTF(_T(" open device %s\r\n"), pDevIface->dbcc_name);
					fDatRes = Open(
								m_szDevName,
								m_DetBaudRate,
								m_DetByteSize,
								m_DetParity,
								m_DetStopBits,
								NPort::DTRControl_Enable,
								NPort::RTSControl_Enable
								);
					if (fDatRes) {
						OnDeviceChange(true);
					}
				}
			}
			break;

		case DBT_DEVICEREMOVECOMPLETE:
			if (pDevIface) 
			{
				if (lstrcmpi(m_szDevName, pDevIface->dbcc_name) == 0) 
				{
					DBG_PRINTF(_T(" close device %s\r\n"), pDevIface->dbcc_name);
					fDatRes = Close();
					if (fDatRes)
						OnDeviceChange(false);
				}
			}
			break;

		default:
			DBG_PRINTF(_T("%s: WM_DEVICECHANGE: 0x%X (%d)\r\n"), FUNCT_NAME_STR, wParm);
			break;
		}
		fRes = true;
		break;

	default:
		break;
	}

	return fRes;
}

bool NPort::EnableAutoDetection(
								const TCHAR *szComName, 
								unsigned int uiBaudRate, 
								NPort::ByteSize eByteSize, 
								NPort::Parity eParity, 
								NPort::StopBits eStopBits
								)
/*
	Desc.   : 
		Enable auto detection mode. When connected this class will open automatically the specified serial port in
		m_szDevName buffer and when disconnected this mechanism will close it automatically. This only usefull when
		serial (COM) port device is a dynamic device like USB Serial converter.
	Params. : 
		szComName
			Serial (COM) port name to be detected. This name should be something like : 
			\\\\?\\USB#Vid_XXX&Pid_YYYY#<SerialNumber>#{a5dcbf10-6530-11d2-901f-00c04fb951ed} where
			XXXX is VID and YYYY is PID and <SerialNumber> is USB Serial number.
		uiBaudRate
			Serial (COM) port baud rate.
		eByteSize
			Byte data size, one of NPort::ByteSize enumeration.
		eParity
			Parity type, one of NPort::Parity enumeration.
		eStopBits
			Stop bits, one of NPort::StopBits enumeration.
	Return  : true if success or otherwise false.
*/
{

	bool	bRes = false;
	bool	bDevRes = false;

	if (m_hNotif) return false;
	if (m_hDev != INVALID_HANDLE_VALUE) return false;
	if (m_hDetTh) return false;

	bRes = SuppressError;
	SuppressError = true;
	
	RtlZeroMemory(m_szDevName, sizeof(m_szDevName));
	StringCbCopy(m_szDevName, sizeof(m_szDevName), szComName);
	m_DetBaudRate	= uiBaudRate;
	m_DetByteSize	= eByteSize;
	m_DetParity		= eParity;
	m_DetStopBits	= eStopBits;

	m_fEnableAutoDet = true;

	bDevRes = Open(
				m_szDevName, 
				m_DetBaudRate, 
				m_DetByteSize, 
				m_DetParity, 
				m_DetStopBits, 
				NPort::DTRControl_Enable, 
				NPort::RTSControl_Enable
				);
	
	SuppressError = bRes;

	if ((m_dwLastError == ERROR_ACCESS_DENIED) && 
		(bDevRes == false))
	{
		RtlZeroMemory(m_szDevName, sizeof(m_szDevName));
		m_fEnableAutoDet = false;
		_HandleError(_T("Can't open device"), m_dwLastError);
		return false;
	}

	RtlZeroMemory(&m_szDevName, sizeof(m_szDevName));
	StringCbCopy(m_szDevName, sizeof(m_szDevName), szComName);

	m_hDetTh = CreateThread(
							NULL,
							0,
							(LPTHREAD_START_ROUTINE) NPort::_DoDetection,
							reinterpret_cast<LPVOID>(this),
							0,
							NULL
							);
	if (!m_hDetTh) {
		m_fEnableAutoDet = false;
		Close();
	} else {
		OnDeviceChange(bDevRes);
	}

	return m_fEnableAutoDet;
}

DWORD CALLBACK NPort::_DoDetection(LPVOID pDat)
/*
	Desc.   : Create message only window and start the detection via its WNDPROC.
	Params. : 
		pDat
			Pointer to data 'this' of current instance if this class.
	Return  : (thread exit code).
*/
{
	NPort *		pThis = reinterpret_cast<NPort *>(pDat);
	WNDCLASSEX	wcx = {0};
	MSG			sMsg = {0};
	HANDLE		hTh = NULL;
	DWORD		dwErr = ERROR_SUCCESS;

	if (!pThis) {
		dwErr = ERROR_INVALID_DATA;
		goto EndDetect;
	}

	wcx.cbSize			= sizeof(WNDCLASSEX);
	wcx.lpfnWndProc		= reinterpret_cast<WNDPROC>(NPort::_DetectProc);
	wcx.lpszClassName	= pThis->m_szDetWndClassName;

	ATOM	at = RegisterClassEx(&wcx);
	if (at <= 0){
		dwErr = GetLastError();
		pThis->m_dwLastError = dwErr;
		goto EndDetect;
	}

	pThis->m_hDetWnd = CreateWindowEx(
									0,
									pThis->m_szDetWndClassName,
									NULL,
									0,
									0,
									0,
									0,
									0,
									HWND_MESSAGE,
									NULL,
									NULL,
									NULL
									);
	if (!pThis->m_hDetWnd) {
		dwErr = GetLastError();
		UnregisterClass(pThis->m_szDetWndClassName, NULL);
		pThis->m_dwLastError = dwErr;
		goto EndDetect;
	}

	DEV_BROADCAST_DEVICEINTERFACE	hdr;
	RtlZeroMemory(&hdr, sizeof(DEV_BROADCAST_DEVICEINTERFACE));
	hdr.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	hdr.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	pThis->m_hNotif = RegisterDeviceNotification(
												pThis->m_hDetWnd, 
												(LPVOID) &hdr, 
												(DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES)
												);
	if (pThis->m_hNotif == NULL)
	{
		dwErr = GetLastError();
		DestroyWindow(pThis->m_hDetWnd);
		pThis->m_dwLastError = dwErr;
		goto Cleanup;
	}

	while (GetMessage(&sMsg, NULL, 0, 0) > 0) {
		TranslateMessage(&sMsg);
		DispatchMessage(&sMsg);
	}

Cleanup:
	UnregisterClass(pThis->m_szDetWndClassName, NULL);

	pThis->m_fEnableAutoDet = false;

	InterlockedExchangePointer((LPVOID) &(pThis->m_hDetWnd), NULL);
	
EndDetect:
	hTh = (HANDLE) InterlockedExchangePointer((LPVOID) & (pThis->m_hDetTh), NULL);
	if (hTh) {
		CloseHandle(hTh);
	}

	ExitThread(dwErr);
	return dwErr;
}

void NPort::DisableAutoDetection()
/*
	Desc.   : Disable the auto detection.
	Params. : None.
	Return  : None.
*/
{
	HANDLE	hTh = NULL;
	DWORD	dwWait;

	if (m_fEnableAutoDet) {
		if (m_hDetWnd) {
			SendMessage(m_hDetWnd, WM_CLOSE, 0, 0);
		}

		hTh = (HANDLE) InterlockedExchangePointer((LPVOID) &(m_hDetTh), NULL);
		if (hTh) {
			dwWait = WaitForSingleObject(hTh, 1000);
			if (dwWait == WAIT_TIMEOUT) {
				TerminateThread(hTh, ERROR_OPERATION_ABORTED);
			}
			CloseHandle(hTh);
		}

		Close();
		RtlZeroMemory(m_szDevName, sizeof(m_szDevName));
		m_fEnableAutoDet = false;
	}
}

HANDLE NPort::_GetHandle()
/*
	Desc.   : 
		Return a handle of opened serial port device. This is getter function coresponds to 
		Handle property.
	Params. : None.
	Return  : Handle to serial port device.
*/
{
	return m_hDev;
}

unsigned int NPort::_GetTimeOut()
/*
	Desc.   : 
		Return time out value. This is getter function in relation to TimeOut property.
	Params. : None.
	Return  : Time out value.
*/
{
	if (m_hDev == INVALID_HANDLE_VALUE)
		return m_uiReadIntervalTimeOut;

	COMMTIMEOUTS	cto = {0};
	BOOL	fRes = GetCommTimeouts(m_hDev, &cto);
	
	return static_cast<unsigned int>(cto.ReadIntervalTimeout);
}

void NPort::_SetTimeOut(unsigned int uiTimeOut)
/*
	Desc.   : Setter function in relation to TimeOut property.
	Params. : 
		uiTimeOut
			Time out interval value to be set to serial port.
	Return  : None.
*/
{
	m_uiReadIntervalTimeOut = uiTimeOut;
	
	if (m_hDev == INVALID_HANDLE_VALUE) 
		return;

	COMMTIMEOUTS	cto = {0};
	BOOL	fRes = GetCommTimeouts(m_hDev, &cto);

	m_uiReadIntervalTimeOut = uiTimeOut;
	cto.ReadIntervalTimeout = m_uiReadIntervalTimeOut;
	fRes = SetCommTimeouts(m_hDev, &cto);
}

unsigned int NPort::_GetOutputQueueSize()
/*
	Desc.   : Getter function in relation to OutputQueueSize property.
	Params. : None.
	Return  : Size of output queue.
*/
{
	return m_uiOutQueueSize;
}

void NPort::_SetOutputQueueSize(unsigned int uiQueueSize)
/*
	Desc.   : Setter function in relation to OutputQueueSize property.
	Params. : 
		uiQueueSize
			Output queue size of serial port.
	Return  : None.
*/
{
	m_uiOutQueueSize = uiQueueSize;

	if (m_hDev != INVALID_HANDLE_VALUE) {
		BOOL	fRes = SetupComm(m_hDev, m_uiInQueueSize, m_uiOutQueueSize);
	}
}

unsigned int NPort::_GetInputQueueSize()
/*
	Desc.   : Getter function in relation to InputQueueSize property.
	Params. : None.
	Return  : Input queue size of serial port.
*/
{
	return m_uiInQueueSize;
}

void NPort::_SetInputQueueSize(unsigned int uiQueueSize)
/*
	Desc.   : Setter function in relation to InputQueueSize property.
	Params. : 
		uiQueueSize
			Input queue size for serial port.
	Return  : None.
*/
{
	m_uiInQueueSize = uiQueueSize;

	if (m_hDev != INVALID_HANDLE_VALUE) {
		BOOL	fRes = SetupComm(m_hDev, m_uiInQueueSize, m_uiOutQueueSize);
	}
}

bool NPort::_GetIsOpen()
/*
	Desc.   : 
		Getter function in relation to IsOpen property, open or close status of current serial port.
	Params. : None.
	Return  : true if serial port is opened or otherwise false.
*/
{
	if (m_hDev == INVALID_HANDLE_VALUE) return false;

	DCB		dcb;
	BOOL	fRes = FALSE;
	RtlZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	__try {
		fRes = GetCommState(m_hDev, &dcb);
	}
	__finally
	{
		// Do nothing
	}

	return ((fRes == TRUE) ? true : false);
}

void NPort::_SetNullStripping(bool fEnable)
/*
	Desc.   : 
		Setter function in relation to NullStripping property, enable or disable null stripping of serial
		port.
	Params. : 
		fEnable
			true to enable null stripping or otherwise false.
	Return  : None.
*/
{
	m_fNullStripping = fEnable;

	if (m_hDev != INVALID_HANDLE_VALUE) {
		DCB		dcb;
		RtlZeroMemory(&dcb, sizeof(DCB));
		dcb.DCBlength = sizeof(DCB);
		
		BOOL fRes = GetCommState(m_hDev, &dcb);
		if (fRes) {
			dcb.fNull = (m_fNullStripping) ? 1 : 0;
			fRes = SetCommState(m_hDev, &dcb);
		}
	}
}

bool NPort::_GetNullStripping()
/*
	Desc.   : 
		Getter function in relation to NullStripping property, enable or disable null stripping of serial
		port.
	Params. : None.
	Return  : true if null stripping is enabled or otherwise false.
*/
{
	if (m_hDev == INVALID_HANDLE_VALUE)
		return m_fNullStripping;

	DCB	dcb;
	RtlZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	BOOL	fRes = GetCommState(m_hDev, &dcb);
	if (fRes) {
		m_fNullStripping = ((dcb.fNull) ? true : false);
	}

	return m_fNullStripping;
}

const TCHAR * NPort::_GetDeviceName()
/*
	Desc.   : Getter function in relation to DeviceName property.
	Params. : None.
	Return  : Null terminated string of serial port device name.
*/
{
	if (lstrlen(m_szDevName) == 0)
		return _T("");

	return &m_szDevName[0];
}

bool NPort::_GetReadEvent()
/*
	Desc.   : 
		Getter function in relation to ReadEvent property, enable or disable read event (OnDataReceived 
		event).
	Params. : None.
	Return  : true if read event is enabled or otherwise false.
*/
{
	return m_bReadEvent;
}

void NPort::_SetReadEvent(bool fEnable)
/*
	Desc.   : 
		Setter function in relation to ReadEvent property, enable or disable read event (OnDataReceived 
		event).
	Params. : 
		fEnable
			true to enable ReadEvent or otherwise false.
	Return  : None.
*/
{
	BOOL		fRes = FALSE;

	if (fEnable) {
		if (m_bReadEvent == false) {
			m_bReadEvent = true;
			if (m_hDev != INVALID_HANDLE_VALUE) {
				m_bStopRead = false;
				m_hReadThread = CreateThread(
											NULL, 
											0, 
											reinterpret_cast<LPTHREAD_START_ROUTINE>(&NPort::_Read), 
											reinterpret_cast<LPVOID>(this), 
											0, 
											NULL
											);
				if (!m_hReadThread) {
					m_bReadEvent = false;
					m_bStopRead = true;
				}
			}
		}
	} else {
		if (m_bReadEvent == true) {
			m_bReadEvent = false;
			if (m_hDev != INVALID_HANDLE_VALUE) {
				fRes = TerminateThread(m_hReadThread, ERROR_OPERATION_ABORTED);
				CloseHandle(m_hReadThread);
				m_hReadThread = NULL;
				m_bStopRead = true;
			}
		}
	}
}

bool NPort::_GetFormatReadData()
/*
	Desc.   : 
		Getter function in relation to FormatReadData property, enable or disable formatted data on read 
		event (OnDataReceived). This property call read event handler with length of read data.
	Params. : None.
	Return  : true if formatted data on read event is enabled or otherwise false.
*/
{
	return m_fFormatedReadData;
}

void NPort::_SetFormatReadData(bool fEnable)
/*
	Desc.   : 
		Setter function in relation to FormatReadData property, enable or disable formatted data on read 
		event (OnDataReceived). This property call read event handler with length of read data.
	Params. : 
		fEnable
			true to enable formatted data on read event or otherwise false.
	Return  : None.
*/
{
	EnterCriticalSection(&m_CritSec);
	m_fFormatedReadData = fEnable;
	LeaveCriticalSection(&m_CritSec);
}

DWORD NPort::_GetLastError()
{
	return m_dwLastError;
}
