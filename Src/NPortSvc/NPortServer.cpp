/*
 * File    : NPortServer.cpp
 * Remark  : Implementation of NPortServer class.
 *
 */

#include "NService.h"
#include "NPortServer.h"
#include "..\NTerminal\Debug.h"
#include "..\NTerminal\NPortName.h"

/*
 * BEGIN NStateLock implementation
 */
NStateLock::NStateLock():
	m_lLock(0),
	m_lCounter(0),
	m_lAcqCnt(0),
	m_lRelCnt(0)
{
}

NStateLock::~NStateLock()
{
}

bool NStateLock::Initialize()
{
	InterlockedExchange(&m_lLock, 0);
	InterlockedExchange(&m_lCounter, 0);

	return true;
}

void NStateLock::Set()
{
	InterlockedExchange(&m_lCounter, 0);
	InterlockedExchange(&m_lLock, 1);
}

void NStateLock::Set(bool bResetCounter)
{
	if (bResetCounter) 
		InterlockedExchange(&m_lCounter, 0);

	InterlockedExchange(&m_lLock, 1);
}

bool NStateLock::Acquire()
{
	LONG l = InterlockedCompareExchange(&m_lLock, 0, 0);
	if (l == 0) return false;
	m_lAcqCnt = InterlockedIncrement(&m_lCounter);
	return true;
}

void NStateLock::Release()
{
	m_lRelCnt = InterlockedDecrement(&m_lCounter);
}

void NStateLock::Reset()
{
	InterlockedExchange(&m_lLock, 0);
}

void NStateLock::ResetAndWait()
{
	LONG	l = 0;

	InterlockedExchange(&m_lLock, 0);
	while (TRUE) {
		l = InterlockedCompareExchange(&m_lCounter, 0, 0);
		if (l == 0) break;
	}
}

LONG NStateLock::GetAcquireCounter()
{
	return m_lAcqCnt;
}

LONG NStateLock::GetReleaseCounter()
{
	return m_lRelCnt;
}

bool NStateLock::Initialize(PNSTATE_LOCK pLock)
{
	if (pLock == NULL) return false;
	
	InterlockedExchange(&(pLock->Lock), 0);
	InterlockedExchange(&(pLock->Counter), 0);
	
	return true;
}

void NStateLock::Set(PNSTATE_LOCK pLock)
{
	if (!pLock) return;

	InterlockedExchange(&(pLock->Counter), 0);
	InterlockedExchange(&(pLock->Lock), 1);
}

bool NStateLock::Acquire(PNSTATE_LOCK pLock)
{
	if (!pLock) return false;
	if (InterlockedCompareExchange(&(pLock->Lock), 0, 0) == 0) return false;
	InterlockedIncrement(&(pLock->Counter));
	return true;
}

void NStateLock::Release(PNSTATE_LOCK pLock)
{
	if (!pLock) return;
	InterlockedDecrement(&(pLock->Counter));
}

void NStateLock::ResetAndWait(PNSTATE_LOCK pLock)
{
	if (!pLock) return;
	InterlockedExchange(&(pLock->Lock), 0);
	while (InterlockedCompareExchange(&(pLock->Counter), 0, 0) != 0);
}
/*
 * END of NStateLock implementation
 */

/*
 * BEGIN NPortServer implementation
 */
volatile bool NPortServer::m_bStopAcceptIo	= true;

NPortServer::NPortServer():
	m_hAcceptIoTh(NULL),
	m_dwErr(0),
	m_hNPort(INVALID_HANDLE_VALUE),
	m_bIsNPortOpened(false)
{
	m_szPortName = NPORT_DEVICE_NAME;
	m_StateLock.Initialize();
}

NPortServer::~NPortServer()
{
	_Stop();
}

void NPortServer::_TerminateThread(PLONG pHandle, DWORD dwMSecTimeOut)
{
	DWORD	dwRes = 0;
	HANDLE	hTh = 0;

	if (!pHandle) return;
	hTh = (HANDLE) InterlockedExchangePointer(pHandle, NULL);
	if (hTh) {
		dwRes = WaitForSingleObject(hTh, dwMSecTimeOut);
		if (dwRes != WAIT_FAILED)
		{
			TerminateThread(hTh, ERROR_OPERATION_ABORTED);
		}
		CloseHandle(hTh);
	}
}

bool NPortServer::_Stop()
{
	_CloseNPort();

	if (!m_bStopAcceptIo) 
		m_bStopAcceptIo = true;

	_TerminateThread((PLONG)&m_hAcceptIoTh, 2000);

	m_MapFile.Destroy();
	m_Synch.Destroy();

	return true;
}

const TCHAR * NPortServer::GetDeviceName()
{
	return m_szPortName;
}

DWORD NPortServer::GetError()
{
	return m_dwErr;
}

void NPortServer::ResetError()
{
	m_dwErr = ERROR_SUCCESS;
}

bool NPortServer::_IsPortExist()
{
	HANDLE	h = NULL;

	h = CreateFile(m_szPortName, GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (h == INVALID_HANDLE_VALUE) return false;

	CloseHandle(h);
	return true;
}

bool NPortServer::Start()
{
	bool	bRes = false;
	ULONG	ulInitEvent = 0;
	ULONG	ulReqEvt = 0;

	bRes = _IsPortExist();
	m_bConnected = bRes;
	ulInitEvent = ((bRes) ? NPROTO_EVENT__DEV_ARRIVAL : NPROTO_EVENT__DEV_REMOVAL);

	do {
		bRes = m_Synch.Create();
		if (!bRes) {
			m_dwErr = m_Synch.GetError();
			break;
		}

		bRes = m_MapFile.Create();
		if (!bRes) {
			m_dwErr = m_MapFile.GetError();
			break;
		}

		bRes = m_Synch.AcquireLock();
		if (!bRes) {
			m_dwErr = m_Synch.GetError();
			break;
		}

		bRes = m_MapFile.Initialize(ulInitEvent);
		if (!bRes) {
			m_dwErr = m_MapFile.GetError();
		}

		m_Synch.ReleaseLock();

		if (m_dwErr != ERROR_SUCCESS) break;

		m_bStopAcceptIo = false;
		m_hAcceptIoTh = CreateThread(
							NULL, 
							0, 
							reinterpret_cast<LPTHREAD_START_ROUTINE>(&NPortServer::_AcceptIo), 
							reinterpret_cast<LPVOID>(this), 
							0, 
							NULL
							);
		if (!m_hAcceptIoTh) {
			m_bStopAcceptIo = false;
			m_dwErr = GetLastError();
			break;
		}

		m_dwErr = ERROR_SUCCESS;
	} while (FALSE);

	if (m_dwErr != ERROR_SUCCESS)
	{
		_Stop();
	}
	else
	{
		m_StateLock.Set();
		SetDeviceEvent(m_bConnected);
	}

	return ((m_dwErr == ERROR_SUCCESS) ? true : false);
}

bool NPortServer::Stop()
{
#ifdef _DEBUG
	LONG lAcq = m_StateLock.GetAcquireCounter();
	LONG lRel = m_StateLock.GetReleaseCounter();
	DbgPrintf(_T("%s: Acq.=%d, Rel.=%d\r\n"), FUNCT_NAME_STR, lAcq, lRel);
#endif

	_CloseNPort();
	m_StateLock.ResetAndWait();

	bool	bRes = false;

	bRes = m_Synch.AcquireLock();
	m_MapFile.Uninitialize();
	m_Synch.ReleaseLock();

	if (!bRes) {
		DBG_PRINTF(_T("%s: Warning: m_Synch.AcquireLock() failed!\r\n"), FUNCT_NAME_STR);
	}

	return _Stop();
}

bool NPortServer::Suspend()
{
	DWORD	dwRes = 0;

	DBG_PRINTF(_T("%s\r\n"), FUNCT_NAME_STR);

	m_StateLock.Reset();

	return ((dwRes != (DWORD) -1) ? true : false);
}

bool NPortServer::Resume()
{
	DWORD	dwRes = 0;

	DBG_PRINTF(_T("%s\r\n"), FUNCT_NAME_STR);

	m_StateLock.Set(false);

	return ((dwRes != (DWORD) -1) ? true : false);
}

DWORD CALLBACK NPortServer::_AcceptIo(LPVOID pDat)
{
	NPortServer *		pThis = reinterpret_cast<NPortServer *>(pDat);
	DWORD				dwRet = 0;
	HANDLE				hTh = NULL;
	bool				bRes = false;
	ServerSynch::Request	Req;

	if (!pThis) {
		dwRet = ERROR_INVALID_DATA;
		goto EndThread;
	}

	DBG_PRINTF(_T("%s: Start.\r\n"), FUNCT_NAME_STR);

	while (!m_bStopAcceptIo)
	{
		bRes = pThis->m_Synch.WaitForReadWriteRequest(500, Req);
		if (!bRes) {
			continue;
		}

		switch (Req)
		{
		case ServerSynch::Req_Read:
			pThis->_HandleReadRequest();
			break;

		case ServerSynch::Req_Write:
			pThis->_HandleWriteRequest();
			break;

		default:
			DBG_PRINTF(_T("%s: Unknown event %d\r\n"), FUNCT_NAME_STR, Req);
			break;
		}
	}

EndThread:
	if (pThis) {
		LPVOID p = InterlockedExchangePointer((PLONG) &(pThis->m_hAcceptIoTh), NULL);
		hTh = reinterpret_cast<HANDLE>(p);
		if (hTh) {
			CloseHandle(hTh);
		}
	}

	DBG_PRINTF(_T("%s: Terminate.\r\n"), FUNCT_NAME_STR);

	ExitThread(dwRet);
	return dwRet;
}

bool NPortServer::SetDeviceEvent(bool fArrival)
{
	bool	bRes = false;
	ULONG	ulCount = 0;
	HANDLE	hOpenTh = NULL;
	DWORD	dwRes = 0;

	m_bConnected = fArrival;

	if (fArrival)
	{
		DBG_PRINTF(_T("%s: Device arrival.\r\n"), FUNCT_NAME_STR);
		
		bRes = m_StateLock.Acquire();
		if (!bRes) return false;

		hOpenTh = CreateThread(
						NULL,
						0,
						reinterpret_cast<LPTHREAD_START_ROUTINE>(&NPortServer::_OpenNPort),
						reinterpret_cast<LPVOID>(this),
						0,
						NULL);
		if (hOpenTh) {
			dwRes = WaitForSingleObject(hOpenTh, 1000);
			switch (dwRes) 
			{
			case WAIT_TIMEOUT:
				DBG_PRINTF(_T("%s: can open port, time out!\r\n"), FUNCT_NAME_STR);
				TerminateThread(hOpenTh, ERROR_OPERATION_ABORTED);
				break;

			case WAIT_OBJECT_0:
				GetExitCodeThread(hOpenTh, &dwRes);
				break;

			default:
				break;
			}
			CloseHandle(hOpenTh);
		}

		m_Synch.AcquireDevEventLock(INFINITE, NULL);
		bRes = m_MapFile.SetDevEventsStatus(NPROTO_EVENT__DEV_ARRIVAL, ulCount);
		m_Synch.ReleaseDevEventLock();
		
		if (bRes) {
			m_Synch.ReleaseDevEvent(ulCount, NULL);
			DBG_PRINTF(_T("%s: event count=%d\r\n"), FUNCT_NAME_STR, ulCount);
		}

		m_StateLock.Release();
	}
	else
	{
		DBG_PRINTF(_T("%s: Device removal.\r\n"), FUNCT_NAME_STR);

		bRes = m_StateLock.Acquire();
		if (bRes) {
			m_Synch.AcquireDevEventLock(INFINITE, NULL);
			bRes = m_MapFile.SetDevEventsStatus(NPROTO_EVENT__DEV_REMOVAL, ulCount);
			m_Synch.ReleaseDevEventLock();
			
			if (bRes) {
				m_Synch.ReleaseDevEvent(ulCount, NULL);
				DBG_PRINTF(_T("%s: event count=%d\r\n"), FUNCT_NAME_STR, ulCount);
			}
			m_StateLock.Release();
		}

		_CloseNPort();
	}

	return bRes;
}

bool NPortServer::_HandleReadRequest()
{
	bool			bRes = false;
	DWORD			dwRet = ERROR_SUCCESS;
	BOOL			fRes = FALSE;
	HANDLE			hRdTh = NULL;
	HANDLE			hProc = NULL;
	ULONG			ulPid = 0;
	ULONG			ulTid = 0;
	DWORD			dwExitCode = 0;

	//DBG_PRINTF(_T("%s\r\n"), FUNCT_NAME_STR);

	bRes = m_StateLock.Acquire();
	if (!bRes) {
		dwRet = ERROR_REQUEST_REFUSED;
		m_MapFile.SetReadResult(dwRet, 0);
		m_Synch.SetReadCompletion();
		return false;
	}

	m_dwReadRes = 0;
	hRdTh = CreateThread(
				NULL,
				0,
				reinterpret_cast<LPTHREAD_START_ROUTINE>(NPortServer::_ReadNPort),
				reinterpret_cast<LPVOID>(this),
				0,
				NULL
				);
	if (!hRdTh) {
		dwRet = GetLastError();
	} else {	// if (!hRdTh) else
		dwRet = WaitForSingleObject(hRdTh, 1000);
		switch (dwRet)
		{
		case WAIT_TIMEOUT:
			dwRet = ERROR_OPERATION_ABORTED;
			fRes = TerminateThread(hRdTh, dwRet);
			DBGIF_PRINTF((fRes == FALSE), _T("%s: Warning: TerminateThread() failed! (%d)\r\n"), FUNCT_NAME_STR, GetLastError());
			break;

		case WAIT_OBJECT_0:
			GetExitCodeThread(hRdTh, &dwRet);
			break;

		default:
			dwRet = GetLastError();
			break;
		}

		CloseHandle(hRdTh);
	}

	if (dwRet == 0) {
		m_MapFile.SetReadResult(ERROR_SUCCESS, m_dwReadLen);
	} else {
		m_MapFile.SetReadResult(dwRet, 0);
	}
	m_Synch.SetReadCompletion();

	m_StateLock.Release();

	return ((dwRet == ERROR_SUCCESS) ? true : false);
}

DWORD CALLBACK NPortServer::_ReadNPort(LPVOID pContext)
{
	NPortServer *	pThis = reinterpret_cast<NPortServer *>(pContext);
	DWORD			dwRet = 0;
	BOOL			fRes = FALSE;
	bool			bRes = false;
	ULONG			ulReqLen = 0;
	PUCHAR			pBuf = NULL;

	if (!pThis) {
		dwRet = ERROR_INVALID_DATA;
		ExitThread(dwRet);
		return dwRet;
	}

	pBuf = pThis->m_MapFile.GetReadBuffer(ulReqLen);
	if (!pBuf) {
		dwRet = pThis->m_MapFile.GetReadError();
		ExitThread(dwRet);
		return dwRet;
	}

	PurgeComm(pThis->m_hNPort, PURGE_RXABORT);

	__try {
		fRes = ReadFile(
				pThis->m_hNPort,
				reinterpret_cast<LPVOID>(pBuf),
				ulReqLen,
				&(pThis->m_dwReadLen),
				NULL
				);
		if (!fRes) {
			dwRet = GetLastError();
			DBG_PRINTF(_T("%s: ReadFile() failed! (%d)\r\n"), FUNCT_NAME_STR, dwRet);
		} else {
			dwRet = ERROR_SUCCESS;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred!!! (0x%X)\r\n"), FUNCT_NAME_STR, dwRet);
	}

	PurgeComm(pThis->m_hNPort, PURGE_RXCLEAR);

	ExitThread(dwRet);
	return dwRet;
}

bool NPortServer::_HandleWriteRequest()
{
	bool			bRes = false;
	DWORD			dwRet = ERROR_SUCCESS;
	BOOL			fRes = FALSE;
	HANDLE			hWrTh = NULL;
	HANDLE			hProc = NULL;
	ULONG			ulPid = 0;
	ULONG			ulTid = 0;
	DWORD			dwExitCode = 0;

	//DBG_PRINTF(_T("%s\r\n"), FUNCT_NAME_STR);

	bRes = m_StateLock.Acquire();
	if (!bRes) {
		dwRet = ERROR_REQUEST_REFUSED;
		m_MapFile.SetWriteResult(dwRet, 0);
		m_Synch.SetWriteCompletion();
		return false;
	}

	m_dwWriteRes = 0;
	hWrTh = CreateThread(
				NULL,
				0,
				reinterpret_cast<LPTHREAD_START_ROUTINE>(NPortServer::_WriteNPort),
				reinterpret_cast<LPVOID>(this),
				0,
				NULL
				);
	if (!hWrTh) {
		dwRet = GetLastError();
	} else {
		dwRet = WaitForSingleObject(hWrTh, 500);
		switch (dwRet)
		{
		case WAIT_TIMEOUT:
			dwRet = ERROR_OPERATION_ABORTED;
			fRes = TerminateThread(hWrTh, dwRet);
			DBGIF_PRINTF((fRes == FALSE), _T("%s: TerminateThread() failed! (%d)\r\n"), FUNCT_NAME_STR, GetLastError());
			break;

		case WAIT_OBJECT_0:
			dwRet = 0;
			GetExitCodeThread(hWrTh, &dwRet);
			break;

		default:
			dwRet = GetLastError();
			break;
		}

		CloseHandle(hWrTh);
	}

	if (dwRet != 0) {
		m_MapFile.SetWriteResult(dwRet, 0);
	} else {
		m_MapFile.SetWriteResult(ERROR_SUCCESS, m_dwWriteLen);
	}
	m_Synch.SetWriteCompletion();

	m_StateLock.Release();

	return ((dwRet == ERROR_SUCCESS) ? true : false);
}

DWORD CALLBACK NPortServer::_WriteNPort(LPVOID pContext)
{
	NPortServer *	pThis = reinterpret_cast<NPortServer *>(pContext);
	DWORD			dwRet = 0;
	BOOL			fRes = FALSE;
	ULONG			ulReqLen = 0;
	bool			bRes = false;
	PUCHAR			pBuf = NULL;

	if (!pThis) {
		dwRet = ERROR_INVALID_DATA;
		ExitThread(dwRet);
		return dwRet;
	}

	pBuf = pThis->m_MapFile.GetWriteBuffer(ulReqLen);
	if (!pBuf) {
		dwRet = pThis->m_MapFile.GetWriteError();
		ExitThread(dwRet);
		return dwRet;
	}

	PurgeComm(pThis->m_hNPort, PURGE_TXABORT);

	__try {
		fRes = WriteFile(
					pThis->m_hNPort,
					reinterpret_cast<LPCVOID>(pBuf),
					ulReqLen,
					&(pThis->m_dwWriteLen),
					NULL
					);
		if (!fRes) {
			dwRet = GetLastError();
			DBG_PRINTF(_T("%s: WriteFile() failed! (%d)\r\n"), FUNCT_NAME_STR, dwRet);
		} else {
			dwRet = ERROR_SUCCESS;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dwRet = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred!!! (0x%X)\r\n"), FUNCT_NAME_STR, dwRet);
	}

	PurgeComm(pThis->m_hNPort, PURGE_TXCLEAR);

	ExitThread(dwRet);
	return dwRet;
}

DWORD CALLBACK NPortServer::_OpenNPort(LPVOID pContext)
{
	NPortServer *	pThis = reinterpret_cast<NPortServer *>(pContext);
	DWORD			dwRet = ERROR_SUCCESS;
	DCB				dcb = {0};
	COMMTIMEOUTS	cto = {0};
	BOOL			fRes = FALSE;

	if (!pThis) {
		dwRet = ERROR_INVALID_DATA;
		ExitThread(dwRet);
		return dwRet;
	}

	do {
		pThis->m_hNPort = CreateFile(
							pThis->m_szPortName, 
							GENERIC_ALL, 
							0, 
							NULL, 
							OPEN_EXISTING, 
							0, 
							NULL
							);
		if (pThis->m_hNPort == INVALID_HANDLE_VALUE) {
			dwRet = GetLastError();
			DBG_PRINTF(_T("%s: CreateFile() failed! (%d)\r\n"), FUNCT_NAME_STR, dwRet);
			break;
		}

		pThis->m_bIsNPortOpened = true;

		fRes = SetupComm(pThis->m_hNPort, 1024, 1024);
		if (!fRes) {
			dwRet = GetLastError();
			DBG_PRINTF(_T("%s: SetupComm() failed! (%d)\r\n"), FUNCT_NAME_STR, dwRet);
			break;
		}

		dcb.DCBlength		= sizeof(DCB);
		dcb.ByteSize		= 8;
		dcb.BaudRate		= 921600;
		dcb.Parity			= NOPARITY;
		dcb.StopBits		= ONESTOPBIT;
		dcb.fDtrControl		= DTR_CONTROL_ENABLE;
		dcb.fRtsControl		= RTS_CONTROL_ENABLE;
		fRes = SetCommState(pThis->m_hNPort, &dcb);
		if (!fRes) {
			dwRet = GetLastError();
			DBG_PRINTF(_T("%s: SetCommState() failed! (%d)\r\n"), FUNCT_NAME_STR, dwRet);
			break;
		}

		cto.ReadIntervalTimeout = 1;
		fRes = SetCommTimeouts(pThis->m_hNPort, &cto);
		if (!fRes) {
			dwRet = GetLastError();
			DBG_PRINTF(_T("%s: SetCommTimeouts() failed! (%d)\r\n"), FUNCT_NAME_STR, dwRet);
			break;
		}

		fRes = TRUE;
	} while (FALSE);

	if (!fRes) 
	{
		pThis->_CloseNPort();
	}

	ExitThread(dwRet);
	return dwRet;
}

void NPortServer::_CloseNPort()
{
	if (!m_bIsNPortOpened) return;

	m_bIsNPortOpened = false;

	BOOL	fRes = FALSE;
	__try {
		fRes = CloseHandle(m_hNPort);
		DBGIF_PRINTF((fRes == FALSE), _T("%s: CloseHandle() failed! (%d)\r\n"), FUNCT_NAME_STR, GetLastError());
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwErr);
		m_hNPort = INVALID_HANDLE_VALUE;
	}
}
/*
 * END of NPortServer implementation
 */
