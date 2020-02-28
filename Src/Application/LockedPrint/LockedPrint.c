/*
 * File    : LockedPrint.c
 * Remark  : Demonstrate thread synchronization with lock. First lock uses ldrex/strex pair and the secound uses NanoOS API.
 *
 */

#include "..\NanoOSApi.h"

UINT32_T		sLock = 0;

/*
 * Acquire lock by using ldrex/strex pair, this is adopted from the manual.
 */
void AcquireLock(UINT32_PTR_T pLock)
{
	__asm volatile (
		"TryLock: MOV.W	R2, %0;"
		"MOV.W R1, #0x1;"		// initialize the ‘lock taken’ value try
		"LDREX R0, [R2];"		// load the lock value
		"CMP R0, #0;"			// is the lock free?
		"ITT EQ;"				// IT instruction for STREXEQ and CMPEQ
		"STREXEQ R0, R1, [R2];"	// try and claim the lock
		"CMPEQ R0, #0;"			// did this succeed?
		"BNE TryLock;"			// no – try again
		: : "r" (pLock)
		);
}

/*
 * Release lock by using ldrex/strex pair, this is adopted from manual.
 */
void ReleaseLock(UINT32_PTR_T pLock)
{
	__asm volatile (
		"TryUnlock: MOV.W R2, %0;"
		"MOV.W R1, #0x0;"		// initialize the ‘lock taken’ value try
		"LDREX R0, [R2];"		// load the lock value
		"CMP R0, #1;"			// is the lock free?
		"ITT EQ;"				// IT instruction for STREXEQ and CMPEQ
		"STREXEQ R0, R1, [R2];"	// try and claim the lock
		"CMPEQ R0, #0;"			// did this succeed?
		"BNE TryUnlock;"		// no – try again
		: : "r" (pLock)
		);
}

/*
 * Thread entry demonstrate print format without lock.
 */
int NoLockedPrint(void *pParm)
{
	UINT32_T	uMax = 100;
	UINT32_T	uiTID = 0;

	GetCurrentThreadId(&uiTID);
	for (int i = 0; i < uMax; i++) {
		Printf("[TID=%d]%s: Test non-locked printf. (%d)\r\n", uiTID, __FUNCTION__, i);
	}

	return uMax;
}

/*
 * Thread entry demonstrate print format with first lock (AcquireLock and ReleaseLock)
 */
int LockedPrint(void *pParm)
{
	UINT32_T	uMax = 100;
	UINT32_T	uiTID = 0;

	GetCurrentThreadId(&uiTID);
	for (int i = 0; i < uMax; i++) {
		AcquireLock(&sLock);
		Printf("[TID=%d]%s: Test locked printf. (%d)\r\n", uiTID, __FUNCTION__, i);
		ReleaseLock(&sLock);

		SwitchToNextThread();
	}

	return uMax;
}

/*
 * Thread entry demonstrate print format with NanoOS API lock: AcquireSpinLock() and ReleaseSpinLock() pair.
 */
int LockedPrintEx(void *pParm)
{
	UINT32_T	uMax = 100;
	UINT32_T	uiTID = 0;

	GetCurrentThreadId(&uiTID);
	for (int i = 0; i < uMax; i++) {
		AcquireSpinLock(&sLock);
		Printf("[TID=%d]%s: Test locked printf. (%d)\r\n", uiTID, __FUNCTION__, i);
		ReleaseSpinLock(&sLock);
	}

	return uMax;
}

int main(int argc, char * argv[])
{
	UINT32_T	uiTID = 0;
	UINT32_T	uiTIDChild = 0;

	/*
	 * Two print threads without lock.
	 */
	Printf("\r\n*** TEST: Print without lock ***\r\n");
	Printf("Begin print without  lock after 2 secs...\r\n");
	Sleep(2000);
	CreateThread(THREAD_CONTROL__UNPRIVILEGED, FALSE, NoLockedPrint, 0, &uiTID);
	CreateThread(THREAD_CONTROL__UNPRIVILEGED, FALSE, NoLockedPrint, 0, &uiTIDChild);
	
	WaitForObject(THREAD_WAIT_OBJ__THREAD, uiTID, THREAD__INFINITE_WAIT);
	WaitForObject(THREAD_WAIT_OBJ__THREAD, uiTIDChild, THREAD__INFINITE_WAIT);

	/*
	 * Two print threads with first lock, ldrex/strex instruction pair.
	 */
	Printf("\r\n*** TEST: Print with lock ***\r\n");
	Printf("Begin print with lock after 2 secs...\r\n");
	Sleep(2000);
	
	sLock = 0;
	CreateThread(THREAD_CONTROL__UNPRIVILEGED, FALSE, LockedPrint, 0, &uiTID);
	CreateThread(THREAD_CONTROL__UNPRIVILEGED, FALSE, LockedPrint, 0, &uiTIDChild);

	WaitForObject(THREAD_WAIT_OBJ__THREAD, uiTID, THREAD__INFINITE_WAIT);
	WaitForObject(THREAD_WAIT_OBJ__THREAD, uiTIDChild, THREAD__INFINITE_WAIT);

	/*
	 * Two print threads with lock from NanoOS API, AcquireSpinLock() and ReleaseSpinLock() pair.
	 */
	Printf("\r\n*** TEST: Print with spin lock ***\r\n");
	Printf("Begin print with spin lock after 2 secs...\r\n");
	Sleep(2000);
	
	InitializeSpinLock(&sLock);
	CreateThread(THREAD_CONTROL__UNPRIVILEGED, FALSE, LockedPrintEx, 0, &uiTID);
	CreateThread(THREAD_CONTROL__UNPRIVILEGED, FALSE, LockedPrintEx, 0, &uiTIDChild);

	WaitForObject(THREAD_WAIT_OBJ__THREAD, uiTID, THREAD__INFINITE_WAIT);
	WaitForObject(THREAD_WAIT_OBJ__THREAD, uiTIDChild, THREAD__INFINITE_WAIT);

	Printf("Program end.\r\n");

	return 0;
}
