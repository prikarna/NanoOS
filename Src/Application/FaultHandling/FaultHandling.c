/*
 * File    : FaultHandling.c
 * Remark  : Demonstrate how to handle fault(s) in NanoOS application. This application uses SetProbe()/ResetProbe()
 *           of NanoOS API to 'simulate' or 'mimic' try/finally logic.
 */

#include "..\NanoOSApi.h"

UINT32_T	gVar = 0xFEEDABEE;

/* Display fault(s) */
void ShowFault(UINT32_T uFault)
{
	UINT32_T	uBusFaultAddr = 0;
	UINT32_T	uMemFaultAddr = 0;
	BOOL		fRes = FALSE;

	Printf("Fault 0x%X:\r\n", uFault);
	if (uFault & TERM__NO_CO_PROCESSOR_FAULT) {
		Printf(" - No co. processor fault.\r\n");
	}
	if (uFault & TERM__INVALID_PC_LOAD_FAULT) {
		Printf(" - Invalid PC load fault.\r\n");
	}
	if (uFault & TERM__INVALID_STATE_FAULT) {
		Printf(" - Invalid state fault.\r\n");
	}
	if (uFault & TERM__UNDEF_INSTRUCTION_FAULT) {
		Printf(" - Undefine instruction fault.\r\n");
	}
	
	if (uFault & TERM__BUS_EXC_ENTRY_STACKING_ERROR) {
		Printf(" - Bus exception entry stacking error fault.\r\n");
	}
	if (uFault & TERM__BUS_EXC_RETURN_STACKING_ERROR) {
		Printf(" - Bus exception return stacking error fault.\r\n");
	}
	if (uFault & TERM__IMPRECISE_ERROR_DATA_FAULT) {
		Printf(" - Imprecise error data fault.\r\n");
	}
	if (uFault & TERM__PRECISE_ERROR_DATA_FAULT) {
		Printf(" - Precise error data fault:\r\n");
		fRes = GetFaultAddress(uFault, &uBusFaultAddr, &uMemFaultAddr);
		if (fRes) {
			Printf("   - Bus fault address = 0x%X\r\n", uBusFaultAddr);
			Printf("   - Mem. fault address = 0x%X\r\n", uMemFaultAddr);
		} else {
			Printf("   - [Not available]\r\n");
		}
	}
	if (uFault & TERM__INSTRUCTION_ERROR_FAULT) {
		Printf(" - Instruction error fault.\r\n");
	}

	if (uFault & TERM__MEM_EXC_ENTRY_STACKING_ERROR) {
		Printf(" - Memory exception entry stacking error fault.\r\n");
	}
	if (uFault & TERM__MEM_EXC_RETURN_STACKING_ERROR) {
		Printf(" - Memory exception return stacking error fault.\r\n");
	}
	if (uFault & TERM__DATA_ACCESS_VIOLATION_FAULT) {
		Printf(" - Data access violation fault.\r\n");
	}
	if (uFault & TERM__INSTRUCTION_ACCESS_VIOLATION_FAULT) {
		Printf(" - Instruction access violation.\r\n");
	}
	if (uFault & TERM__USER_REQUEST) {
		Printf(" - User request.\r\n");
	}
	if (uFault & TERM__STACK_REACH_ITS_LIMIT_FAULT) {
		Printf(" - Stack reach its limit (threshold).\r\n");
	}
}

int main(int argc, char * argv[])
{
	UINT32_PTR_T	pFaultAddr = (UINT32_PTR_T) 0x1FFFFFFC;
	UINT32_T		uProbeStat = 0;
	BOOL			fSuccess = FALSE;
	UINT32_PTR_T	pValidAddr = &gVar;

	Printf("\r\n***TEST: handling fault.\r\n");

	Printf("Set probe... ");

	/* Call SetProbe() */
	uProbeStat = SetProbe();
	
	/*
	 * When SetProbe() fail the application just return -1 (exit)
	 */
	if (uProbeStat == PROBE_STATUS__ERROR) {
		Printf("Failed! (%d)\r\n", GetLastError());
		return -1;
	}

	/*
	 * When SetProbe() return PROBE_STATUS__FAULT this mean a fault occured while executing another part of this thread,
	 * the thread simply go or jump to label 'Cleanup' to execute a 'finalizing' codes.
	 */
	if (uProbeStat == PROBE_STATUS__FAULT) {
		goto Cleanup;
	}

	/*
	 * When SetProbe() return PROBE_STATUS__ORIGIN this mean call to SetProbe() success and this is the first return,
	 * thread simply continue to execute the codes below. When a fault occured while executing codes below thread will 
	 * 'jump back' to where SetProbe() is about to return and SetProbe() return PROBE_STATUS__FAULT.
	 */
	if (uProbeStat == PROBE_STATUS__ORIGIN) {
		Printf("Success.\r\n");
	} else {
		Printf("Success but unexpected probe status. (stat=%d)\r\n", uProbeStat);
		return -2;
	}

	/*
	 * These three lines of codes (not including blank line) below demonstrate access to invalid address of this chipset
	 * or demonstrate a failure or fault.
	 */
	Printf("Try to access invalid address (0x%X)... ", pFaultAddr);
	
	UINT32_T	u;

	u = *pFaultAddr;

	/*
	 * These three lines of codes (not including blank line) below demonstrate access to valid address of this chipset
	 * or demonstrate success. To demonstrate a success, comment out those three lines of codes above and then uncomment
	 * these three lines below.
	 */
	//Printf("Try to access valid address (0x%X)... ", pValidAddr);
	//
	//UINT32_T	u;

	//u = *pValidAddr;

	/*
	 * If no fault occured while executing lines of codes above, application will continue to execute these lines below.
	 */
	fSuccess = TRUE;		// To indicate success operation.
	ResetProbe(FALSE);		// Unset or clear probe of current thread with FALSE parameter (no return).

	/*
	 * Cleanup or finallizing codes
	 */
Cleanup:
	if (!fSuccess) {	// If fault(s) occured.
		u = GetLastFault();
		Printf("fail!\r\n");
		ShowFault(u);
	} else {			// If no fault occured (success operation).
		Printf("success. Content = 0x%X\r\n", u);
	}

	Printf("Program end.\r\n");

	return 0;
}