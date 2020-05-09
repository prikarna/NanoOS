/*
 * File    : NService.h"
 * Remark  : Provide abstraction for service class.
 *
 */

#pragma once

#include <vector>

#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"
#include "Dbt.h"

//
//	Desc.:
//		Provide abstract class for service service class, can't be used directly. A service class
//		must inherit from this class so NServiceDispatcher load can call function implemented in 
//		this class
//
//	
class NService
{
public:

	NService();
	~NService();

	virtual bool	HandleStarting(DWORD dwArgc, LPTSTR * lpszArgv) = 0;
	virtual bool	HandleStopping() = 0;
	virtual bool	HandlePausing() = 0;
	virtual bool	HandleContinuing() = 0;
	virtual bool	HandleDeviceArrival(PDEV_BROADCAST_DEVICEINTERFACE pDevIface) = 0;
	virtual bool	HandleDeviceRemoval(PDEV_BROADCAST_DEVICEINTERFACE pDevIface) = 0;

	DWORD			GetError();

protected:
	DWORD			m_dwError;

private:
};

//
//	Desc.:
//		Provide a the dispatcher for for service object, load and then run. A service can request to 
//		pend the state changing if required.
//		
//
//
class NServiceDispatcher
{
public:
	enum Request {
		Req_StartPending = 1,
		Req_StopPending,
		Req_PausePending,
		Req_ContinuePending
	};

	NServiceDispatcher();
	~NServiceDispatcher();

	bool			Run(NService *pService);
	static bool		RequestPending(NServiceDispatcher::Request request, unsigned long ulMilliSeconds, unsigned long ulError);

private:
	static TCHAR					m_szName[256];
	static TCHAR					m_szDisplayName[256];
	static SERVICE_STATUS_HANDLE	m_hSvcStat;
	static HDEVNOTIFY				m_hNotify;
	static HANDLE					m_hStopEvt;
	static SERVICE_STATUS			m_Status;
	static DWORD					m_dwCheckPoint;
	static DWORD					m_dwControlsAccept;

	static NService	*				m_pService;

	static void WINAPI	_Main(DWORD dwArgc, LPTSTR * lpszArgv);
	static DWORD WINAPI	_HandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);

	static bool			_Report(DWORD dwCurrentState, DWORD dwExitCode, DWORD dwWaitHint);
};
