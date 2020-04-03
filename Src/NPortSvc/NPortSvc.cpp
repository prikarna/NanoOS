/*
 * File    : NPortSvc.cpp
 * Remark  : NPortSvc class implementation.
 *
 */

#include "NPortSvc.h"
#include "..\NTerminal\Debug.h"

NPortSvc::NPortSvc()
{
	m_szDevName = const_cast<TCHAR *>(m_Server.GetDeviceName());
}

NPortSvc::~NPortSvc()
{
}

bool NPortSvc::HandleStarting(DWORD dwArgc, LPTSTR * lpszArgv)
{
	bool	bRes = true;

	NServiceDispatcher::RequestPending(NServiceDispatcher::Req_StartPending, 1000, 0);
	bRes = m_Server.Start();
	if (!bRes) {
		m_dwError = m_Server.GetError();
	}
	
	return bRes;
}

bool NPortSvc::HandleStopping()
{
	bool	bRes = true;

	NServiceDispatcher::RequestPending(NServiceDispatcher::Req_StopPending, 2000, 0);
	m_Server.ResetError();
	bRes = m_Server.Stop();
	if (!bRes) {
		m_dwError = m_Server.GetError();
	}

	return bRes;
}

bool NPortSvc::HandleContinuing()
{
	bool	bRes = true;

	NServiceDispatcher::RequestPending(NServiceDispatcher::Req_StartPending, 2000, 0);
	m_Server.ResetError();
	bRes = m_Server.Resume();
	if (!bRes) {
		m_Server.Stop();
	}

	return bRes;
}

bool NPortSvc::HandlePausing()
{
	bool	bRes = true;

	NServiceDispatcher::RequestPending(NServiceDispatcher::Req_StartPending, 2000, 0);
	m_Server.ResetError();
	bRes = m_Server.Suspend();
	if (!bRes) {
		m_Server.Stop();
	}

	return bRes;
}

bool NPortSvc::HandleDeviceArrival(PDEV_BROADCAST_DEVICEINTERFACE pDevIface)
{
	bool	bRes = true;
	
	if (pDevIface) {
		if (lstrcmpi(m_szDevName, pDevIface->dbcc_name) == 0)
		{
			m_Server.SetDeviceEvent(true);
		}
	}

	return bRes;
}

bool NPortSvc::HandleDeviceRemoval(PDEV_BROADCAST_DEVICEINTERFACE pDevIface)
{
	bool	bRes = true;

	if (pDevIface) {
		if (lstrcmpi(m_szDevName, pDevIface->dbcc_name) == 0)
		{
			m_Server.SetDeviceEvent(false);
		}
	}

	return bRes;
}
