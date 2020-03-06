#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"

#include "..\NTerminal\NPort.h"
#include "NPortLib.h"

static NPort			sPort;
static const TCHAR *	sszPortName = _T("\\\\?\\USB#Vid_6a16&Pid_0230#09092019#{a5dcbf10-6530-11d2-901f-00c04fb951ed}");
static NPL_CONNECTION_CHANGE_CALLBACK		sConnChangeCallback = 0;
static NPL_DATA_RECEIVED_CALLBACK			sDatRecvCallback = 0;
static NPL_ERROR_CALLBACK					sErrorCallback = 0;

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID )
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		break;

	case DLL_PROCESS_DETACH:
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
	char * szErrorMessage;

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

	(* sErrorCallback)(szErrorMessage);
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

#ifdef __cplusplus
}
#endif
