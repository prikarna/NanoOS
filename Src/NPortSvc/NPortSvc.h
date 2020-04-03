/*
 * File    : NPortSvc.h
 * Remark  : NService 'realization' class.
 *
 */

#include "NService.h"
#include "NPortServer.h"

class NPortSvc: public NService
{
public:
	NPortSvc();
	~NPortSvc();

	bool HandleStarting(DWORD dwArgc, LPTSTR * lpszArgv);
	bool HandleStopping();
	bool HandlePausing();
	bool HandleContinuing();
	bool HandleDeviceArrival(PDEV_BROADCAST_DEVICEINTERFACE pDevIface);
	bool HandleDeviceRemoval(PDEV_BROADCAST_DEVICEINTERFACE pDevIface);

private:
	TCHAR *		m_szDevName;
	NPortServer	m_Server;
};
