/*
 * File    : NPortLib.cpp
 * Remark  : NPortLib implementation.
 *
 */

#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"

#include "..\NTerminal\NPort.h"
#include "..\NTerminal\NInstaller.h"
#include "..\NTerminal\NPortName.h"

#include "NPortLib.h"

#ifdef _DEBUG
# define DBG_PRINTF(szFormat, ...)			DbgPrintf(szFormat, __VA_ARGS__)
#else
# define DBG_PRINTF(szFormat, ...)
#endif

static NPort			sPort;
static NInstaller		sInstaller;
static bool				sfInstalling = false;
static const TCHAR *	sszPortName = NPORT_DEVICE_NAME;

static NPL_CONNECTION_CHANGE_CALLBACK		sConnChangeCallback = 0;
static NPL_DATA_RECEIVED_CALLBACK			sDatRecvCallback = 0;
static NPL_ERROR_CALLBACK					sErrorCallback = 0;
static NPL_INSTALLING_CALLBACK				sInstallCallback = 0;

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID )
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		DBG_PRINTF(_T("%s: DLL_PROCESS_ATTACH\r\n"), FUNCT_NAME_STR);
		break;

	case DLL_PROCESS_DETACH:
		DBG_PRINTF(_T("%s: DLL_PROCESS_DETACH\r\n"), FUNCT_NAME_STR);
		break;

	default:
		break;
	}

	return 1;
}

void HandleDeviceChange(bool fIsConnected)
{
	unsigned char uIsConn;

	if (sConnChangeCallback == 0) return;

	uIsConn = (fIsConnected) ? 1 : 0;
	(* sConnChangeCallback)(uIsConn);
}

void HandleFormattedReadData(NPort::ReadDataFormat * pReadData)
{
	unsigned char * pDat;
	int				iDatLen;

	if (sDatRecvCallback == 0) return;

	pDat = static_cast<unsigned char *>(pReadData->RawData);
	iDatLen = static_cast<int>(pReadData->DataLength);
	(* sDatRecvCallback)(pDat, iDatLen);
}

void HandleError(const TCHAR * szErrMsg)
{
	char *			szErrorMessage;
	unsigned long	ulErrCode;

	if (sErrorCallback == 0) return;

#ifdef UNICODE
	int	iRes;
	char buf[512] = {0};

	iRes = WideCharToMultiByte(
							CP_ACP, 
							WC_COMPOSITECHECK,
							szErrMsg,
							-1,
							buf,
							sizeof(buf),
							NULL,
							NULL
							);
	if (iRes <= 0) return;
	szErrorMessage = &buf[0];
#else
	szErrorMessage = const_cast<char *>(szErrMsg);
#endif

	if (sfInstalling) {
		ulErrCode = sInstaller.LastError;
		sfInstalling = false;
	} else {
		ulErrCode = sPort.LastError;
	}

	(* sErrorCallback)(szErrorMessage, ulErrCode);
}

void HandleInstalling(int iProgress)
{
	if (sInstallCallback) {
		if (iProgress == 100)
			sfInstalling = false;

		(* sInstallCallback)(iProgress);
	}
}

#ifdef __cplusplus
extern "C" {
#endif

NPORTLIB
int NPL_Open(
		 NPL_CONNECTION_CHANGE_CALLBACK ConnectionChangeCallback,
		 NPL_DATA_RECEIVED_CALLBACK DataReceivedCallback,
		 NPL_ERROR_CALLBACK ErrorCallback
		 )
{
	if (ConnectionChangeCallback) {
		sConnChangeCallback = ConnectionChangeCallback;
		sPort.OnDeviceChange = HandleDeviceChange;
	}

	if (DataReceivedCallback) {
		sDatRecvCallback = DataReceivedCallback;
		sPort.OnFormatedReadData = HandleFormattedReadData;
		sPort.ReadEvent = true;
		sPort.FormatedReadData = true;
	} else {
		sPort.ReadEvent = false;
		sPort.FormatedReadData = false;
	}

	if (ErrorCallback) {
		sErrorCallback = ErrorCallback;
		sPort.OnError = HandleError;
	}

	bool bRes = false;
	bRes = sPort.EnableAutoDetection(
								sszPortName,
								921600,
								NPort::ByteSize_8,
								NPort::Parity_None,
								NPort::StopBits_1
								);
	if (!bRes) {
		sConnChangeCallback	= 0;
		sDatRecvCallback	= 0;
		sErrorCallback		= 0;
	}

	return ((bRes) ? 1 : 0);
}

NPORTLIB void NPL_Close()
{
	sPort.DisableAutoDetection();

	sConnChangeCallback	= 0;
	sDatRecvCallback	= 0;
	sErrorCallback		= 0;
}

NPORTLIB
int NPL_Write(unsigned char * pBuffer, int iBufferLength)
{
	bool	bRes = false;

	bRes = sPort.Write(pBuffer, static_cast<unsigned int>(iBufferLength));

	return ((bRes) ? 1 : 0);
}

NPORTLIB
int NPL_Read(unsigned char * pBuffer, int iBufferLength, int * piRead)
{
	if (sDatRecvCallback) return 0;

	bool			bRes = false;
	unsigned int	uiRead;
	bRes = sPort.Read(pBuffer, static_cast<unsigned int>(iBufferLength), uiRead);
	if (!bRes) return 0;

	if (piRead) *piRead = static_cast<int>(uiRead);
	return 1;
}

NPORTLIB
int NPL_IsOpen()
{
	bool bRes = sPort.IsOpen;

	return ((bRes) ? 1 : 0);
}

NPORTLIB
unsigned long NPL_GetErrorCode()
{
	DWORD	dw = 0;
	if (sfInstalling) {
		dw = sInstaller.LastError;
	} else {
		dw = sPort.LastError;
	}
	return dw;
}

NPORTLIB
int NPL_Purge(enum NPL_PurgeOption Option)
{
	NPort::PurgeOption	Opt;
	switch (Option)
	{
	case NPLPurge_RxAbort:
		Opt = NPort::PurgeOption_RxAbort;
		break;

	case NPLPurge_RxClear:
		Opt = NPort::PurgeOption_RxClear;
		break;

	case NPLPurge_TxAbort:
		Opt = NPort::PurgeOption_TxAbort;
		break;

	case NPLPurge_TxClear:
		Opt = NPort::PurgeOption_TxClear;
		break;

	case NPLPurge_RxTxAbort:
		Opt = NPort::PurgeOption_RxTxAbort;
		break;

	case NPLPurge_RxTxClear:
		Opt = NPort::PurgeOption_RxTxClear;
		break;

	case NPLPurge_All:
		Opt = NPort::PurgeOption_All;
		break;

	default:
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

	bool b = sPort.Purge(Opt);
	return ((b) ? 1 : 0);
}

NPORTLIB
int NPL_InstallNApplication(
		const char * szApplicationFile,
		NPL_INSTALLING_CALLBACK InstallingCallback
		)
{
#ifdef UNICODE
	WCHAR *	szFileName = NULL;
	int	iRes;
	WCHAR buf[1024] = {0};

	iRes = MultiByteToWideChar(
							CP_ACP, 
							MB_COMPOSITE,
							szApplicationFile,
							-1,
							buf,
							(sizeof(buf) / sizeof(WCHAR))
							);
	if (iRes <= 0) return 0;
	szFileName = &buf[0];
#else
	char * szFileName = NULL;
	szFileName = szApplicationFile;
#endif

	sInstallCallback = InstallingCallback;
	sInstaller.OnInstalling = HandleInstalling;
	sInstaller.OnError = HandleError;

	sfInstalling = true;
	bool	bRes = sInstaller.Install(&sPort, szFileName);
	if (!bRes) sfInstalling = false;

	return ((bRes) ? 1 : 0);
}

NPORTLIB
int NPL_IsInstalling()
{
	bool	bRes = sInstaller.IsInstalling;
	if (bRes != sfInstalling) sfInstalling = bRes;

	return ((bRes) ? 1 : 0);
}

NPORTLIB
int NPL_CancelInstall()
{
	bool	bRes = sInstaller.Cancel();
	if (bRes) sfInstalling = false;

	return ((bRes) ? 1 : 0);
}

#ifdef __cplusplus
}
#endif

/*
 * Begin export function for NanoOS Port service
 */
#include "..\NPortSvc\NPortClient.h"

static NPLSVC_CONNECTION_CHANGE_CALLBACK	sSvcConnChange = 0;
static NPortClient	Port;

void _SvcHandleConnChange(bool fConnected)
{
	if (sSvcConnChange) {
		int iConn = (fConnected) ? 1 : 0;
		(sSvcConnChange)(iConn);
	}
}

#ifdef __cplusplus
extern "C" {
#endif

NPORTLIB
int NPLSVC_Open(NPLSVC_CONNECTION_CHANGE_CALLBACK ConnChangeCallback)
{
	Port.ConnectionChanged	= _SvcHandleConnChange;
	sSvcConnChange			= ConnChangeCallback;

	bool	bRes = Port.Open();
	return ((bRes) ? TRUE : FALSE);
}

NPORTLIB void NPLSVC_Close()
{
	Port.Close();
}

NPORTLIB
int NPLSVC_Write(const unsigned char * pData, int iDataLength)
{
	bool bRes = Port.Write(pData, iDataLength);
	return ((bRes) ? 1 : 0);
}

NPORTLIB
int NPLSVC_Read(unsigned char * pBuffer, int iBufferLength, int * piReadLength)
{
	unsigned int uiRead = 0;
	bool bRes = Port.Read(pBuffer, iBufferLength, uiRead);
	if (bRes) {
		if (piReadLength) {
			*piReadLength = uiRead;
		}
	}

	return ((bRes) ? 1 : 0);
}

NPORTLIB
int NPLSVC_WriteThenRead(
					 const unsigned char * pWriteData, 
					 int iWriteDataLength,
					 unsigned char * pReadBuffer,
					 int iReadBufferLength,
					 int * piReadLength
					 )
{
	unsigned int uiReadLen = 0;
	bool bRes = Port.WriteThenRead(
					pWriteData,
					iWriteDataLength,
					pReadBuffer,
					iReadBufferLength,
					uiReadLen
					);
	if (bRes) {
		if (piReadLength) {
			*piReadLength = uiReadLen;
		}
	}

	return ((bRes) ? 1 : 0);
}

NPORTLIB
int NPLSVC_ReadThenWrite(
					 unsigned char * pReadBuffer,
					 int iReadBufferLength,
					 int * piReadLength,
					 const unsigned char * pWriteData,
					 int iWriteDataLength
					 )
{
	unsigned int uiReadLen = 0;
	bool bRes = Port.ReadThenWrite(
						pReadBuffer,
						iReadBufferLength,
						uiReadLen,
						pWriteData,
						iWriteDataLength
						);
	if (bRes) {
		if (piReadLength) {
			*piReadLength = uiReadLen;
		}
	}

	return ((bRes) ? 1 : 0);
}

NPORTLIB
unsigned long NPLSVC_GetLastError()
{
	return Port.GetError();
}
/*
 * End export function for NanoOS Port service
 */

#ifdef __cplusplus
}
#endif
