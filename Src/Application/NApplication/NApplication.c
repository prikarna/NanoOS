/*
 * File    : NApplication.c
 * Remark  : Simple sample to demonstrate the usage of NanoOS API.
 *
 */

#include "..\NanoOSApi.h"

int			giUnInitVar;
int			giInitVar = 0xFEEDABEE;
const int	gciConst = 0xBEBADA55;
UINT32_T	guiTID = 0;

int SimpleThread(void * pParam)
{
	UINT32_T	uCount, uSpin;
	UINT32_T	uMax = 60;
	UINT32_T	uTID = 0;

	if (pParam) {
		uMax = *((UINT32_PTR_T) pParam);
	}

	GetCurrentThreadId(&uTID);
	Printf("[TID=%d]%s: Startin.. (uMax = %d)\r\n", uTID, __FUNCTION__, uMax);

	for (uCount = 0; uCount < uMax; uCount++)
	{
		Printf("[TID=%d]%s: Count = %d\r\n", uTID, __FUNCTION__, uCount);
		uSpin = 100000;
		while (uSpin-- > 0);
	}

	return uCount;
}

int TimeConsumingThread(void * pParm)
{
	UINT32_T	uEvtId = 0;
	UINT32_T	u;
	BOOL		fRes;
	UINT32_T	uiTID;

	if (!pParm) return 0;

	GetCurrentThreadId(&uiTID);

	uEvtId = *((UINT32_PTR_T) pParm);

	for (u = 0; u < 5; u++) 
	{
		Printf("[TID=%d]%s: Work %d...", uiTID, __FUNCTION__, u);
		fRes = Sleep(1000);
		Printf(" done.\r\n", 0);
	}

	fRes = SetEvent(uEvtId);
	if (!fRes) {
		Printf("[TID=%d]%s: SetEvent failed!\r\n", uiTID, __FUNCTION__);
	}

	Printf("[TID=%d]%s: exit.\r\n", uiTID, __FUNCTION__);
	return 0;
}

int WakeUpThread(void * pParm)
{
	UINT32_T	uTargetTID = 0;
	UINT32_T	u;
	BOOL		fRes;
	UINT32_T	uTID;

	if (!pParm) return 0;

	uTargetTID = *((UINT32_PTR_T) pParm);
	GetCurrentThreadId(&uTID);

	Printf("[TID=%d]%s: Wake up thread id %d in 5 secs... ", uTID, __FUNCTION__, uTargetTID);
	for (u = 1; u <= 5; u++) 
	{
		fRes = Sleep(1000);
		Printf("%d ",u);
	}
	fRes = ResumeThread(uTargetTID);
	if (!fRes) {
		u = GetLastError();
		Printf("Failed! (0x%X)\r\n", u);
	} else {
		Printf("\r\n", 0);
	}

	return 0;
}

int main(int argc, char * argv[])
{
	UINT32_T	uiTID = 0;
	char		szOSName[64];
	UINT32_T	uiVer = 0;
	UINT32_T	uClk = 0;
	BOOL		fRes = FALSE;
	UINT32_T	uiTIDChild = 0;
	UINT8_T		uiStatus;
	int			iRetVal;
	UINT32_T	uEvtId;
	UINT32_T	uErr;

	for (int i = 0; i < 10; i++) {
		fRes = IsUsbSerialReady();
		if (fRes) break;
		Sleep(500);
	}

	if (!fRes) {
		DBG_PRINTF("USB serial not ready, exiting!\r\n", 0);
		return -1;
	};

	Printf("\r\nTest vars.: giUnInitVar=%d, giInitVar=0x%X, gciConst=0x%X\r\n", giUnInitVar, giInitVar, gciConst);

	MemSet((UINT8_PTR_T) szOSName, 0, sizeof(szOSName));
	Printf("Get system clock... ", 0);
	fRes = GetSystemClockSpeed(&uClk);
	if (fRes)
		Printf("OK.\r\n", 0);
	else
		Printf("Failed!\r\n", 0);
	
	Printf("Get OS name... ", 0);
	fRes = GetOSName(szOSName, sizeof(szOSName));
	if (fRes)
		Printf("OK.\r\n", 0);
	else
		Printf("Failed!\r\n", 0);

	Printf("Get OS version... ", 0);
	fRes = GetOSVersion(&uiVer);
	if (fRes)
		Printf("OK.\r\n", 0);
	else
		Printf("Failed!\r\n", 0);

	Printf("Get current thread id... ", 0);
	fRes = GetCurrentThreadId(&uiTID);
	if (fRes)
		Printf("OK.\r\n", 0);
	else
		Printf("Failed!\r\n", 0);

	Printf("\r\n", 0);
	Printf("OS Name           = %s\r\n", szOSName);
	Printf("OS Version        = %d.%d\r\n", ((uiVer >> 8) & 0xFF), (uiVer & 0xFF));
	Printf("System clock      = %d Hz, (%d MHz)\r\n", uClk, (uClk / 1000000));
	Printf("Current thread id = %d\r\n", uiTID);

	/*
	 * Demonstrate simple thread
	 */
	Printf("\r\n*** TEST: Simple thread ***\r\n", 0);
	Printf("Create thread... ", 0);
	fRes = CreateThread(THREAD_CONTROL__UNPRIVILEGED, TRUE, SimpleThread, 0, &uiTIDChild);
	if (!fRes) {
		uErr = GetLastError();
		Printf("Failed! (0x%X)\r\n", uErr);
		return -1;
	}
	Printf("OK (New thread id=%d), now resume this thread and wait...\r\n", uiTIDChild);
	ResumeThread(uiTIDChild);
	while (TRUE)
	{
		fRes = GetThreadStatus(uiTIDChild, &uiStatus);
		if (!fRes) {
			uErr = GetLastError();
			Printf("Fail to get thread state! (%d)\r\n", uErr);
			break;
		}
		if ((uiStatus == THREAD_STATUS__EXIT) ||
			(uiStatus == THREAD_STATUS__TERMINATED))
		{
			GetThreadReturnValue(uiTIDChild, &iRetVal);
			Printf("New thread exit. (status=%d, ret. val.=%d)\r\n", uiStatus, iRetVal);
			break;
		}
	}

	/*
	 * Demonstrate sleep of current thread.
	 */
	Printf("\r\n*** TEST: Sleep 5 times ***\r\n");
	uClk = 1000;
	for (int i = 0; i < 5; i++) {
		Printf("[%d] Sleep approx. 1 sec... ", i);
		fRes = Sleep(uClk);
		if (!fRes) {
			uErr = GetLastError();
			Printf("failed! (0x%X)\r\n", uErr);
			break;
		} else {
			Printf("done.\r\n");
		}
	}
	Printf("\r\n");

	/*
	 * Demonstrate thread synchronization with event object.
	 */
	Printf("\r\n*** TEST: Synchronization ***\r\n");
	Printf("Create event... ");
	fRes = CreateEvent(&uEvtId);
	if (!fRes) {
		uErr = GetLastError();
		Printf("Failed! (%d)\r\n", uErr);
		goto End;
	}
	Printf("OK (event id=%d).\r\n", uEvtId);
	Printf("Create time consuming thread... ");
	fRes = CreateThread(THREAD_CONTROL__UNPRIVILEGED, TRUE, TimeConsumingThread, (void *) &uEvtId, &uiTIDChild);
	if (!fRes) {
		uErr = GetLastError();
		Printf("Failed! (0x%X)\r\n", uErr);
		goto Cleanup;
	}
	Printf("OK (TID=%d), now resume and wait...\r\n", uiTIDChild);
	ResumeThread(uiTIDChild);
	fRes = WaitForObject(THREAD_WAIT_OBJ__EVENT, uEvtId, THREAD__INFINITE_WAIT);
	if (!fRes) {
		uErr = GetLastError();
		Printf("Wait error. (0x%X)\r\n", uErr);
	} else {
		Printf("Wait done.\r\n");
	}

	Sleep(500);

	/*
	 * Demonstrate suspend and resume thread.
	 */
	Printf("\r\n*** TEST: Suspend current thread ***\r\n");
	GetCurrentThreadId(&guiTID);
	Printf("Create wake up thread...\r\n");
	fRes = CreateThread(THREAD_CONTROL__UNPRIVILEGED, FALSE, WakeUpThread, &guiTID, 0);
	if (!fRes) {
		uErr = GetLastError();
		Printf("Failed! (0x%X)\r\n", uErr);
		goto Cleanup;
	}
	fRes = SuspendThread(guiTID);
	if (!fRes) {
		uErr = GetLastError();
		Printf("Wakeup from suspend error! (0x%X)\r\n", uErr);
	} else {
		Printf("Wakeup from suspend.\r\n");
	}

Cleanup:
	CloseEvent(uEvtId);

End:
	Printf("Program end.\r\n", 0);

	return 0;
}


//int main(int argc, char *argv[])
//{
//	UaWriteChar('D');
//	UaWriteChar('e');
//	UaWriteChar('k');
//	UaWriteChar('a');
//	return 0;
//}