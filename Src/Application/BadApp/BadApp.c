/*
 * File    : BadApp.c
 * Remark  : Demonstrate a bad application that uses large amount of stack. NanoOS can handle fault(s) like this application. 
 *           Default action is to terminate the faulty thread without 'bothering' or 'disturbing' any other thread but 
 *           this application set a termination handler, so when fault(s) occured, NanoOS will call the termination handler.
 *
 */

#include "..\NanoOSApi.h"

/* Dummy structure */
typedef struct _A_STRUCT {
	UINT32_T		Dummy0;
	UINT32_T		Dummy1;
	UINT32_T		Dummy2;
	UINT32_T		Dummy3;
	UINT32_T		Dummy4;
	UINT32_T		Dummy5;
	UINT32_T		Dummy6;
	UINT32_T		Dummy7;
	UINT32_T		Dummy8;
	UINT32_T		Dummy9;
	UINT32_T		Dummy10;
	UINT32_T		Dummy11;
	UINT32_T		Dummy12;
	UINT32_T		Dummy13;
	UINT32_T		Dummy14;
	UINT32_T		Dummy15;
} A_STRUCT, *PA_STRUCT;

UINT32_T	guCount = 0;

/* Termination handler, uiReason parameter is the information of what fault(s) causing thread termination */
void AppTermHandler(UINT32_T uiReason)
{
	UINT32_T	uBusFaultAddr = 0;
	UINT32_T	uMemFaultAddr = 0;
	BOOL		fRes = FALSE;

	Printf("\r\nTermiante app.: Reason = 0x%X:\r\n", uiReason);
	if (uiReason & TERM__NO_CO_PROCESSOR_FAULT) {
		Printf(" - No co. processor fault.\r\n");
	}
	if (uiReason & TERM__INVALID_PC_LOAD_FAULT) {
		Printf(" - Invalid PC load fault.\r\n");
	}
	if (uiReason & TERM__INVALID_STATE_FAULT) {
		Printf(" - Invalid state fault.\r\n");
	}
	if (uiReason & TERM__UNDEF_INSTRUCTION_FAULT) {
		Printf(" - Undefine instruction fault.\r\n");
	}
	
	if (uiReason & TERM__BUS_EXC_ENTRY_STACKING_ERROR) {
		Printf(" - Bus exception entry stacking error fault.\r\n");
	}
	if (uiReason & TERM__BUS_EXC_RETURN_STACKING_ERROR) {
		Printf(" - Bus exception return stacking error fault.\r\n");
	}
	if (uiReason & TERM__IMPRECISE_ERROR_DATA_FAULT) {
		Printf(" - Imprecise error data fault.\r\n");
	}
	if (uiReason & TERM__PRECISE_ERROR_DATA_FAULT) {
		Printf(" - Precise error data fault:\r\n");
		fRes = GetFaultAddress(uiReason, &uBusFaultAddr, &uMemFaultAddr);
		if (fRes) {
			Printf("   - Bus fault address = 0x%X\r\n", uBusFaultAddr);
			Printf("   - Mem. fault address = 0x%X\r\n", uMemFaultAddr);
		} else {
			Printf("   - [Not more info]\r\n");
		}
	}
	if (uiReason & TERM__INSTRUCTION_ERROR_FAULT) {
		Printf(" - Instruction error fault.\r\n");
	}

	if (uiReason & TERM__MEM_EXC_ENTRY_STACKING_ERROR) {
		Printf(" - Memory exception entry stacking error fault.\r\n");
	}
	if (uiReason & TERM__MEM_EXC_RETURN_STACKING_ERROR) {
		Printf(" - Memory exception return stacking error fault.\r\n");
	}
	if (uiReason & TERM__DATA_ACCESS_VIOLATION_FAULT) {
		Printf(" - Data access violation fault.\r\n");
	}
	if (uiReason & TERM__INSTRUCTION_ACCESS_VIOLATION_FAULT) {
		Printf(" - Instruction access violation.\r\n");
	}
	if (uiReason & TERM__USER_REQUEST) {
		Printf(" - User request.\r\n");
	}
	if (uiReason & TERM__STACK_REACH_ITS_LIMIT_FAULT) {
		Printf(" - Stack reach its limit (threshold).\r\n");
	}
}

/* 'Forever recurse function' with call to one of NanoOS API */
void BadRecurse(int i)
{
	UINT32_PTR_T	pStack = 0;

	__asm volatile ("MOV.W %0, SP;" : "=r" (pStack));

	Printf("[0x%X] %d ", (UINT32_T) pStack, i++);
	BadRecurse(i);
}

/* 'Forever recurse function' without call to NanoOS API */
void BadRecurse2(int i)
{
	i++;
	BadRecurse2(i);
}

void BadRecurse3(int a, int b, int c, int d)
{
	a++;
	b += 2;
	c += 3;
	d += 4;
	BadRecurse3(a, b, c, d);
}

void BadRecurse4()
{
	guCount++;
	BadRecurse4();
}

/* Function with large stack allocation */
void BadFunc()
{
	A_STRUCT		A1[200];
	A_STRUCT		Dat[200];
	A_STRUCT		A2[500];

	Printf("%s: Enter\r\n", __FUNCTION__);

	A1[2].Dummy0 = 1;
	A1[99].Dummy10 = 10;

	Dat[101].Dummy14 = 14;

	A2[11].Dummy0 = 1;
	A2[12].Dummy0 = 12;
	A2[20].Dummy0 = 20;

	Printf("%s: Dummy: %d, Dummy: %d, Dummy: %d, Dummy: %d\r\n", __FUNCTION__, 
		A1[2].Dummy0, A1[99].Dummy10, Dat[101].Dummy14, A2[11].Dummy0);
}

/* Function with large stack allocation no. 2 (unused) */
//void BadFunc2()
//{
//	A_STRUCT		A1[16];
//	A_STRUCT		A2[4];
//
//	Printf("%s: Enter\r\n", __FUNCTION__);
//
//	MemSet((UINT8_PTR_T) &A1, 0, sizeof(A1));
//	MemSet((UINT8_PTR_T) &A2, 0, sizeof(A2));
//
//	A1[0].Dummy0 = 1;
//	A1[15].Dummy10 = 10;
//
//	A2[1].Dummy0 = 1;
//	A2[3].Dummy11 = 12;
//	A2[7].Dummy15 = 15;		// This line should generate compilation error but it doesn't. A bug ?
//
//	Printf("%s: Dummy: %d, Dummy: %d, Dummy: %d, Dummy: %d\r\n", __FUNCTION__, 
//		A1[0].Dummy0, A1[15].Dummy10, A2[3].Dummy11, A2[7].Dummy15);
//}

int main(int argc, char * argv[])
{
	BOOL			fRes = FALSE;
	
	for (int i = 0; i < 10; i++)
	{
		fRes = IsUsbSerialReady();
		if (fRes) break;

		Sleep(500);
	}
	if (!fRes) {
		DBG_PRINTF("BadApp quit!\r\n", 0);
		return -1;
	}

	Printf("\r\n***TEST: bad application.\r\n");

	Printf("Set termination handler (handler=0x%X)... ", AppTermHandler);
	fRes = SetTerminationHandler(AppTermHandler);
	if (!fRes) {
		Printf("Failed!\r\n");
		goto End;
	}
	Printf("Success.\r\n");

	/* Call 'forever recurse function' */
	BadRecurse(1);

	/*
	 * Comment out the line above and uncomment one of these lines below to change to other faulty situation.
	 */
	//BadRecurse2(1);
	//BadRecurse3(1, 1, 1, 1);
	//BadRecurse4();
	//BadFunc();

End:
	Printf("Program end.\r\n");

	return 0;
}
