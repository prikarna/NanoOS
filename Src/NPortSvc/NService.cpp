/*
 * File    : NService.cpp
 * Remark  : NService class implementation.
 *
 */

#include "NService.h"
#include "..\NTerminal\Debug.h"

/*
 * BEGIN NService implementations
 */
NService::NService():
	m_dwError(0)
{
}

NService::~NService()
{
}

DWORD NService::GetError()
{
	return m_dwError;
}
/*
 * END of NService implementations
 */


/*
 * BEGIN NServiceDispatcher implementations
 */
SERVICE_STATUS_HANDLE	NServiceDispatcher::m_hSvcStat				= NULL;
HDEVNOTIFY				NServiceDispatcher::m_hNotify				= NULL;
HANDLE					NServiceDispatcher::m_hStopEvt				= NULL;
DWORD					NServiceDispatcher::m_dwCheckPoint			= 1;
SERVICE_STATUS			NServiceDispatcher::m_Status				= {0};
DWORD					NServiceDispatcher::m_dwControlsAccept		= (SERVICE_ACCEPT_HARDWAREPROFILECHANGE |
																	   SERVICE_ACCEPT_PAUSE_CONTINUE |
																	   SERVICE_ACCEPT_SESSIONCHANGE |
																	   SERVICE_ACCEPT_POWEREVENT |
																	   SERVICE_ACCEPT_STOP);
TCHAR					NServiceDispatcher::m_szName[256]			= {0};
TCHAR					NServiceDispatcher::m_szDisplayName[256]	= {0};
NService *				NServiceDispatcher::m_pService				= NULL;

NServiceDispatcher::NServiceDispatcher()
{
	RtlZeroMemory(m_szName, sizeof(m_szName));
	StringCbCopy(m_szName, sizeof(m_szName), _T("NPortSvc"));

	RtlZeroMemory(m_szDisplayName, sizeof(m_szDisplayName));
	StringCbCopy(m_szDisplayName, sizeof(m_szDisplayName), _T("NPort Service"));

	RtlZeroMemory(&m_Status, sizeof(SERVICE_STATUS));
	m_Status.dwServiceType		= SERVICE_WIN32_OWN_PROCESS;
}

NServiceDispatcher::~NServiceDispatcher()
{
}

void WINAPI NServiceDispatcher::_Main(DWORD dwArgc, LPTSTR *lpszArgv)
{
	m_hSvcStat = RegisterServiceCtrlHandlerEx(m_szDisplayName, &NServiceDispatcher::_HandlerEx, NULL);
	if (!m_hSvcStat) {
		DBG_PRINTF(_T("%s: RegisterServiceCtrlHandlerEx() failed! (%d)\r\n"), FUNCT_NAME_STR, GetLastError());
		return;
	}

	_Report(SERVICE_START_PENDING, ERROR_SUCCESS, 3000);

	m_hStopEvt = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!m_hStopEvt) {
		DBG_PRINTF(_T("%s: CreateEvent() failed! (%d)\r\n"), FUNCT_NAME_STR, GetLastError());
		_Report(SERVICE_STOPPED, GetLastError(), 0);
		return;
	};

	DEV_BROADCAST_DEVICEINTERFACE	DevIface;
	RtlZeroMemory(&DevIface, sizeof(DEV_BROADCAST_DEVICEINTERFACE));
	DevIface.dbcc_size			= sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	DevIface.dbcc_devicetype	= DBT_DEVTYP_DEVICEINTERFACE;
	m_hNotify = RegisterDeviceNotification(
								m_hSvcStat, 
								(LPVOID) &DevIface, 
								(DEVICE_NOTIFY_SERVICE_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES)
								);
	DBGIF_PRINTF((m_hNotify == NULL), 
		_T("%s: Warning: RegisterDeviceNotification() failed! (%d)\r\n"), FUNCT_NAME_STR, GetLastError());

	bool	bRes = false;
	DWORD	dwErr = 0;

	if (m_pService) {
		bRes = m_pService->HandleStarting(dwArgc, lpszArgv);
		if (bRes) {
			_Report(SERVICE_RUNNING, NO_ERROR, 0);
			WaitForSingleObject(m_hStopEvt, INFINITE);
			if (m_pService) {
				dwErr = m_pService->GetError();
			}
		} else {
			dwErr = m_pService->GetError();
			DBG_PRINTF(_T("%s: Fail to start service! (%d)\r\n"), FUNCT_NAME_STR, dwErr);
		}
	}

	CloseHandle(m_hStopEvt);

	DBG_PRINTF(_T("%s: Service stopped. (%d)\r\n"), FUNCT_NAME_STR, dwErr);

	_Report(SERVICE_STOPPED, dwErr, 0);
	m_hSvcStat = NULL;
}

DWORD WINAPI NServiceDispatcher::_HandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
	DWORD	dwRet = ERROR_CALL_NOT_IMPLEMENTED;
	BOOL	fRes = FALSE;
	PDEV_BROADCAST_DEVICEINTERFACE	pDevIface = NULL;
	PDEV_BROADCAST_HDR				pDevHdr = NULL;
	PDEV_BROADCAST_HANDLE			pDevHandle = NULL;
	DWORD	dwError = NO_ERROR;
	bool	bRes = false;

	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
		_Report(SERVICE_STOP_PENDING, NO_ERROR, 1000);
		if (m_hNotify) {
			fRes = UnregisterDeviceNotification(m_hNotify);
			if (fRes) {
				m_hNotify = NULL;
			}
		}

		if (m_pService) {
			bRes = m_pService->HandleStopping();
		}

		SetEvent(m_hStopEvt);
		dwRet = NO_ERROR;
		break;


	case SERVICE_CONTROL_PAUSE:
		_Report(SERVICE_PAUSE_PENDING, NO_ERROR, 1000);
		if (m_pService) {
			bRes = m_pService->HandlePausing();
			if (!bRes) {
				dwError = m_pService->GetError();
				DBG_PRINTF(_T("%s: Error occured while suspending, stoping... (%d)\r\n"), FUNCT_NAME_STR, dwError);
				m_pService->HandleStopping();
			}
		}
		if (dwError != NO_ERROR) {
			_Report(SERVICE_STOPPED, dwError, 0);
		} else {
			_Report(SERVICE_PAUSED, NO_ERROR, 0);
		}
		dwRet = NO_ERROR;
		break;


	case SERVICE_CONTROL_CONTINUE:
		_Report(SERVICE_CONTINUE_PENDING, NO_ERROR, 1000);
		if (m_pService) {
			bRes = m_pService->HandleContinuing();
			if (!bRes) {
				dwError = m_pService->GetError();
				DBG_PRINTF(_T("%s: Error occured while resuming, stoping... (%d)\r\n"), FUNCT_NAME_STR, dwError);
				m_pService->HandleStopping();
			}
		}
		
		if(dwError != NO_ERROR) {
			_Report(SERVICE_STOPPED, dwError, 0);
		} else {
			_Report(SERVICE_RUNNING, NO_ERROR, 0);
		}
		dwRet = NO_ERROR;
		break;


	case SERVICE_CONTROL_INTERROGATE:
		dwRet = NO_ERROR;
		break;

	case SERVICE_CONTROL_DEVICEEVENT:
		pDevHdr = reinterpret_cast<PDEV_BROADCAST_HDR>(lpEventData);
		if (pDevHdr) {
			if (pDevHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
			{
				pDevIface = reinterpret_cast<PDEV_BROADCAST_DEVICEINTERFACE>(lpEventData);
			}
			else if (pDevHdr->dbch_devicetype == DBT_DEVTYP_HANDLE)
			{
				pDevHandle = reinterpret_cast<PDEV_BROADCAST_HANDLE>(lpEventData);
			}
			else
			{
				// Do nothing
			}
		}
		switch (dwEventType)
		{
		case DBT_DEVICEARRIVAL:
			if (m_pService) m_pService->HandleDeviceArrival(pDevIface);
			break;

		case DBT_DEVICEREMOVECOMPLETE:
			if (m_pService) m_pService->HandleDeviceRemoval(pDevIface);
			break;

		case DBT_DEVICEQUERYREMOVE:
			DBG_PRINTF(_T("%s: SERVICE_CONTROL_DEVICEEVENT: DBT_DEVICEQUERYREMOVE\r\n"), FUNCT_NAME_STR);
			break;

		case DBT_DEVICEQUERYREMOVEFAILED:
			DBG_PRINTF(_T("%s: SERVICE_CONTROL_DEVICEEVENT: DBT_DEVICEQUERYREMOVEFAILED\r\n"), FUNCT_NAME_STR);
			break;

		case DBT_DEVICEREMOVEPENDING:
			DBG_PRINTF(_T("%s: SERVICE_CONTROL_DEVICEEVENT: DBT_DEVICEREMOVEPENDING\r\n"), FUNCT_NAME_STR);
			break;

		case DBT_CUSTOMEVENT:
			DBG_PRINTF(_T("%s: SERVICE_CONTROL_DEVICEEVENT: DBT_CUSTOMEVENT\r\n"), FUNCT_NAME_STR);
			break;

		default:
			DBG_PRINTF(_T("%s: SERVICE_CONTROL_DEVICEEVENT: (dwEventType=0x%X)\r\n"), FUNCT_NAME_STR, dwEventType);
			break;
		}
		dwRet = NO_ERROR;
		break;

	case SERVICE_CONTROL_HARDWAREPROFILECHANGE:
		switch (dwEventType)
		{
		case DBT_CONFIGCHANGED:
			DBG_PRINTF(_T("%s: SERVICE_CONTROL_HARDWAREPROFILECHANGE: DBT_CONFIGCHANGED\r\n"), FUNCT_NAME_STR);
			break;

		case DBT_QUERYCHANGECONFIG:
			DBG_PRINTF(_T("%s: SERVICE_CONTROL_HARDWAREPROFILECHANGE: DBT_QUERYCHANGECONFIG\r\n"), FUNCT_NAME_STR);
			break;

		case DBT_CONFIGCHANGECANCELED:
			DBG_PRINTF(_T("%s: SERVICE_CONTROL_HARDWAREPROFILECHANGE: DBT_CONFIGCHANGECANCELED\r\n"), FUNCT_NAME_STR);
			break;

		default:
			DBG_PRINTF(_T("%s: SERVICE_CONTROL_HARDWAREPROFILECHANGE: (dwEventType=0x%X)\r\n"), FUNCT_NAME_STR, dwEventType);
			break;
		}
		dwRet = NO_ERROR;
		break;

	case SERVICE_CONTROL_POWEREVENT:
		DBG_PRINTF(_T("%s: SERVICE_CONTROL_POWEREVENT\r\n"), FUNCT_NAME_STR);
		dwRet = NO_ERROR;
		break;

	case SERVICE_CONTROL_SESSIONCHANGE:
		DBG_PRINTF(_T("%s: SERVICE_CONTROL_SESSIONCHANGE\r\n"), FUNCT_NAME_STR);
		dwRet = NO_ERROR;
		break;

	default:
		DBG_PRINTF(_T("%s: dwControl=0x%X\r\n"), FUNCT_NAME_STR, dwControl);
		break;
	}

	return dwRet;
}

bool NServiceDispatcher::Run(NService *pService)
{
	SERVICE_TABLE_ENTRY		ste[] =
	{
		{ m_szName, &NServiceDispatcher::_Main },
		{ NULL, NULL }
	};

	m_pService = pService;

	BOOL fRes = StartServiceCtrlDispatcher(ste);

	DBGIF_PRINTF((fRes == FALSE), 
		_T("%s: RegisterServiceCtrlDispatcher() failed! (%d)\r\n"), FUNCT_NAME_STR, GetLastError());

	return ((fRes) ? true : false);
}

bool NServiceDispatcher::_Report(DWORD dwCurrentState, DWORD dwExitCode, DWORD dwWaitHint)
{
	m_Status.dwCurrentState		= dwCurrentState;
	m_Status.dwWaitHint			= dwWaitHint;
	m_Status.dwWin32ExitCode	= dwExitCode;
	m_Status.dwServiceSpecificExitCode	= 0;

	switch (dwCurrentState)
	{
	case SERVICE_CONTINUE_PENDING:
	case SERVICE_PAUSE_PENDING:
	case SERVICE_START_PENDING:
	case SERVICE_STOP_PENDING:
		m_Status.dwControlsAccepted = 0;
		m_Status.dwCheckPoint = m_dwCheckPoint;
		m_dwCheckPoint++;
		break;

	default:
		m_Status.dwControlsAccepted = m_dwControlsAccept;
		m_dwCheckPoint = 1;
		m_Status.dwCheckPoint		= 0;
		break;
	}

	BOOL fRes = SetServiceStatus(m_hSvcStat, &m_Status);
	DBGIF_PRINTF((!fRes), _T("%s: SetServiceStatus() failed! (%d)\r\n"), FUNCT_NAME_STR, GetLastError());

	return ((fRes) ? true : false);
}

bool NServiceDispatcher::RequestPending(NServiceDispatcher::Request request, unsigned long ulMilliSeconds, unsigned long ulError)
{
	DWORD	dwState = 0;

	if (ulMilliSeconds == 0) return false;

	switch (request)
	{
	case NServiceDispatcher::Req_ContinuePending:
		dwState = SERVICE_CONTINUE_PENDING;
		break;

	case NServiceDispatcher::Req_PausePending:
		dwState = SERVICE_PAUSE_PENDING;
		break;

	case NServiceDispatcher::Req_StartPending:
		dwState = SERVICE_START_PENDING;
		break;

	case NServiceDispatcher::Req_StopPending:
		dwState = SERVICE_STOP_PENDING;
		break;

	default:
		return false;
	}

	return _Report(dwState, ulError, ulMilliSeconds);
}
/*
 * END of NServiceDispatcher implementations
 */
