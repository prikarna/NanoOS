/*
 * File    : Core.c
 * Remark  : NanoOS start-up (initialization) and fault exception handlers functions.
 *           NanoOS has ability to handle some faults by:
 *           1. Terminate faulty thread without reset the chipset or 'bothering' 
 *              other active thread(s).
 *           2. Call thread termination handler if thread register it, so it can run 
 *              some clean up code in certain amount of time.
 *           3. 'Simulate' or 'mimic' a try/finally logic by seting probe mode to 
 *              a thread.
 *           For the hard fault NanoOS simply display the fault to console and then reset the chip.
 *
 */

#include "Sys\CortexM\Nvic.h"
#include "Sys\CortexM\Scb.h"
#include "Sys\CortexM\SysTick.h"
#include "Sys\Stm\Gpio.h"
#include "Sys\Stm\Rcc.h"

#include "Type.h"
#include "NanoOS.h"
#include "Thread.h"
#include "Service.h"
#include "Exception.h"

extern unsigned int	_etext, _start_data, _end_data;
extern unsigned int	_start_bss, _end_bss;

extern UINT32_PTR_T		gpLastDbgStack;
extern UINT32_PTR_T		gpLastDbgFrameAddr;

extern PTHREAD	ThdGetCurrent();
extern PTHREAD	ThdGetFirstThread();
extern void		ThdParkThread();

UINT32_PTR_T		gpLastIntStack;
UINT32_T			guBusFaultAddress;
UINT32_T			guMemFaultAddress;
UINT32_T			guAppTID;

static UINT32_PTR_T				spStack;
static UINT32_PTR_T				spTmpStack;
static PINITIAL_THREAD_STACK	spInitStack;
static UINT32_T					suVar;
static UINT32_T					suTID;
static UINT32_T					suStat;
static UINT32_T					suHFStat;
static BOOL						sfRes;
static PCONFIG					spCfg;
static PTHREAD					spCurThread;

static UINT32_T					suTpAddr;
static UINT32_T					suTpCode;
static UINT32_T					suTpParam1;
static UINT32_T					suTpParam2;

#ifndef _DEBUG
static UINT32_T					suCount;
#endif

__attribute__((naked))
void ExcNonMaskableInterrupt()
{
#ifdef _DEBUG
	DbgConsole(TRAP__NMI, 0, 0);
#else
	UsbShutdown();

	RCC_ENA_APB2_CTRL_CLK(ENABLE, RCC_APB2_CTRL__IOPC);

	GPIO_SET_MODE_OUTPUT(GPIO_PORT__C, GPIO_PIN__13, GPIO_CNF_OUTPUT__GEN_PUSH_PULL, GPIO_MODE__OUTPUT_2MHZ);

	for (suVar = 0; suVar < 20; suVar++) {
		GPIO_WRITE(GPIO_PORT__C, GPIO_PIN__13, LOW);
		suCount = 400000;
		while (suCount-- > 0);

		GPIO_WRITE(GPIO_PORT__C, GPIO_PIN__13, HIGH);
		suCount = 400000;
		while (suCount-- > 0);
	}

	SCB_RESET_SYSTEM();
#endif
}

static void CrDisplayFault()
{
	UINT32_T	uFaultAddr = 0;

	if ((suStat & SCB_GENERAL_FAULTS) == 0) 
	{
		UaWriteString("   Unknown faults.\r\n");
		return;
	}

	/* Memory Fault */
	if (suStat & SCB_MEM_FAULT__DATA_ACCESS_VIOLATION)
	{
		UaWriteString("   Memory fault\t\t: Data access violation.\r\n");
	}
	if (suStat & SCB_MEM_FAULT__EXC_ENTRY_STACKING_ERROR)
	{
		UaWriteString("   Memory fault\t\t: Exception ENTRY stacking error.\r\n");
	}
	if (suStat & SCB_MEM_FAULT__EXC_RETURN_STACKING_ERROR) 
	{
		UaWriteString("   Memory fault\t\t: Exception RETURN stacking error.\r\n");
	}
	if (suStat & SCB_MEM_FAULT__INSTRUCTION_ACCESS_VIOLATION) 
	{
		UaWriteString("   Memory fault\t\t: Instruction access violation.\r\n");
	}
	if (suStat & SCB_MEM_FAULT__VALID_FAULT_ADDRESS) 
	{
		uFaultAddr = SCB_GET_MEM_FAULT_ADDRESS();
		UaPrintf("   Memory fault address\t: 0x%X\r\n", uFaultAddr);
	}

	/* Bus fault */
	if (suStat & SCB_BUS_FAULT__EXC_ENTRY_STACKING_ERROR) 
	{
		UaWriteString("   Bus fault\t\t: Exception ENTRY stacking error.\r\n");
	}
	if (suStat & SCB_BUS_FAULT__EXC_RETURN_STACKING_ERROR) 
	{
		UaWriteString("   Bus fault\t\t: Exception RETURN stacking error.\r\n");
	}
	if (suStat & SCB_BUS_FAULT__IMPRECISE_ERROR_DATA) 
	{
		UaWriteString("   Bus fault\t\t: Imprecise error data.\r\n");
	}
	if (suStat & SCB_BUS_FAULT__PRECISE_ERROR_DATA) 
	{
		UaWriteString("   Bus fault\t\t: Precise error data.\r\n");
	}
	if (suStat & SCB_BUS_FAULT__INSTRUCTION_ERROR) 
	{
		UaWriteString("   Bus fault\t\t: Instruction error.\r\n");
	}
	if (suStat & SCB_BUS_FAULT__VALID_FAULT_ADDRESS)
	{	
		uFaultAddr = SCB_GET_BUS_FAULT_ADDRESS();
		UaPrintf("   Bus faulting address\t: 0x%X\r\n", uFaultAddr);
	}

	/* Usage fault */
	if (suStat & SCB_USAGE_FAULT__DEVIDE_BY_ZERO) 
	{
		UaWriteString("   Usage fault\t\t: Devide by zero.\r\n");
	}
	if (suStat & SCB_USAGE_FAULT__UNALIGNED_USAGE) 
	{
		UaWriteString("   Usage fault\t\t: Unaligned usage.\r\n");
	}
	if (suStat & SCB_USAGE_FAULT__NO_CO_PROCESSOR) 
	{
		UaWriteString("   Usage fault\t\t: No co-processor.\r\n");
	}
	if (suStat & SCB_USAGE_FAULT__INVALID_PC_LOAD) 
	{
		UaWriteString("   Usage fault\t\t: Invalid PC load.\r\n");
	}
	if (suStat & SCB_USAGE_FAULT__INVALID_STATE) 
	{
		UaWriteString("   Usage fault\t\t: Invalid state.\r\n");
	}
	if (suStat & SCB_USAGE_FAULT__UNDEF_INSTRUCTION) 
	{
		UaWriteString("   Usage fault\t\t: Undefined instruction.\r\n");
	}
}

static void CrDisplayHardFault()
{
	UINT32_T	uTID;

	UaWriteString("\r\nHARD FAULT\r\n");

	if (ThdGetCurrentId(&uTID)) {
		if (uTID >= MAX_NUMB_OF_THREADS) {
			UaPrintf(" Thread ID\t: <Invalid>\r\n");
		} else {
			UaPrintf(" Thread ID\t: %d\r\n", uTID);
		}
	}
	UaPrintf(" Status\t: 0x%X\r\n", suHFStat);

	if ((suHFStat & SCB_HARDFAULT__ALL) == 0)
	{
		UaWriteString("  Unknown.\r\n");
		return;
	}

	if (suHFStat & SCB_HARDFAULT__DEBUG_VT)
	{
		UaWriteString("  Debug VT fault.\r\n");
	}

	if (suHFStat & SCB_HARDFAULT__VECTOR_TABLE) 
	{
		UaWriteString("  Vector table fault.\r\n");
	}

	if (suHFStat & SCB_HARDFAULT__FORCED) 
	{
		UaPrintf("  Forced (Gen. fault=0x%X)\t:\r\n", suStat);
		CrDisplayFault();
	}
}

__attribute__((naked))
void CrTerminateFaultyThread()
{
	spTmpStack = spCurThread->StartStackPtr;

	spCurThread->LastStackPtr = (spTmpStack - (SIZEOF_INITIAL_STACK() - 1));
	spInitStack = (PINITIAL_THREAD_STACK) spCurThread->LastStackPtr;

	spInitStack->FrameAddress	= (UINT32_T) spTmpStack;
	spInitStack->ExcReturn		= EXC_RETURN;

	if (spCurThread->TermHandler) {
		spCurThread->Flags			|= THREAD_FLAG__TERMINATING;
		spCurThread->Counter		= TERMINATING_COUNT;
		spInitStack->ExcFrame.PC	= (UINT32_T) spCurThread->TermHandler;
		spInitStack->ExcFrame.LR	= (UINT32_T) ThdExit;
		spInitStack->ExcFrame.R0	= suStat;

		spCurThread->TermHandler	= 0;
	} else {
		spCurThread->State			= THREAD_STATE__TERMINATED;
		spInitStack->ExcFrame.PC	= (UINT32_T) ThdParkThread;
	}
	
	spInitStack->ExcFrame.PSR	= BITHEX_24;

	__asm volatile 
		(
		"MSR CONTROL, %0;"
		"MOV.W SP, %1;"
		: : 
			"r" (spCurThread->Control), 
			"r" (spCurThread->LastStackPtr)
		);

	END_EXCEPTION();
}

__attribute__((naked))
void ExcHardFault()
{
	GET_STACK_POINTER(gpLastDbgStack);

	SET_TO_SYSTEM_STACK();

	suHFStat = SCB_GET_HARDFAULT_STATUS();
	suStat = SCB_GET_GENERAL_FAULT_STATUS();
	
	CrDisplayHardFault();

#ifdef _DEBUG
	DbgConsole(TRAP__HARD_FAULT, suHFStat, suStat);
#else
	spCurThread = ThdGetCurrent();
	TRAP_EXEC((spCurThread == 0), TRAP__POSSIBLE_DATA_CORRUPTION, suStat, 0);
	TRAP_EXEC(
		((spCurThread->Id >= MAX_NUMB_OF_THREADS) ||
		(spCurThread->State > THREAD_STATE__SUSPENDED) ||
		(spCurThread->State != THREAD_STATE__RUNNING)),
		TRAP__POSSIBLE_DATA_CORRUPTION,
		suHFStat,
		suStat
		);

	CrTrapExecution(TRAP__HARD_FAULT, suHFStat, suStat);
#endif
}

/*
 * Require more space
 *
__attribute__((naked))
void CrReturnToSetProbe()
{
	sfRes = ThdRestoreCurrentThread();
	if (!sfRes) {
		TermFaultyThread();
	}

	spCurThread->Flags &= ~(THREAD_FLAG__PROBING);

	spStack = spCurThread->LastStackPointer;
	spStack += 2;
	spParm = (PSVC_PARAM) spStack;

	*((UINT32_PTR_T) spParm->Params[0]) = PROBE_STATUS__FAULT;
	spParm->ReturnValue = TRUE;
	spParm->PSR = BITHEX_24;

	__asm volatile 
		(
		"MSR CONTROL, %0;"
		"MOV.W SP, %1;"
		: : 
			"r" (spCurThread->Control), 
			"r" (spCurThread->LastStackPointer)
		);

	END_EXCEPTION();
}
*/

__attribute__((naked))
void CrReturnToSetProbe()
{
	spCurThread->Flags			&= ~(THREAD_FLAG__PROBING);

	spInitStack = (PINITIAL_THREAD_STACK) spCurThread->ProbeStackPtr;
	spInitStack->FrameAddress	= spCurThread->ProbeFrameAddress;
	spInitStack->ExcReturn		= EXC_RETURN;
	spInitStack->ExcFrame.LR	= spCurThread->ProbeLR;
	spInitStack->ExcFrame.PC	= spCurThread->ProbePC;
	spInitStack->ExcFrame.R1	= PROBE_STATUS__FAULT;
	spInitStack->ExcFrame.PSR	= BITHEX_24;

	__asm volatile 
		(
		"MOV.W SP, %0;"
		: : 
			"r" (spInitStack)
		);

	END_EXCEPTION();
}

__attribute__((naked))
void ExcGeneralFault()
{
	GET_STACK_POINTER(gpLastDbgStack);

	SET_TO_SYSTEM_STACK();

	CLEAR_EXCLUSIVE();

	sfRes = ThdGetCurrentId(&suTID);
	suStat = SCB_GET_GENERAL_FAULT_STATUS();

#ifdef _DEBUG
	UaPrintf("\r\n FAULT (0x%X) :\r\n", suStat);
	if (sfRes) {
		if (suTID >= MAX_NUMB_OF_THREADS) {
			UaPrintf("  TID\t\t: <Invalid>\r\n");
		} else {
			UaPrintf("  TID\t\t: %d\r\n", suTID);
		}
	}
	CrDisplayFault();
#endif

	spCurThread = ThdGetCurrent();
	TRAP_EXEC((spCurThread == 0), TRAP__POSSIBLE_DATA_CORRUPTION, suStat, 0);
	TRAP_EXEC(
		((spCurThread->Id >= MAX_NUMB_OF_THREADS) ||
		(spCurThread->State > THREAD_STATE__SUSPENDED) ||
		(spCurThread->State != THREAD_STATE__RUNNING)),
		TRAP__POSSIBLE_DATA_CORRUPTION,
		suStat,
		0
		);

	if (suStat) {
		if (suStat & SCB_BUS_FAULT__VALID_FAULT_ADDRESS) {
			guBusFaultAddress = SCB_GET_BUS_FAULT_ADDRESS();
		} else {
			guBusFaultAddress = 0;
		}
		if (suStat & SCB_MEM_FAULT__VALID_FAULT_ADDRESS) {
			guMemFaultAddress = SCB_GET_MEM_FAULT_ADDRESS();
		} else {
			guMemFaultAddress = 0;
		}
		SCB_CLR_GENERAL_FAULT_STATUS();
	}

	spStack = gpLastIntStack;
	if (gpLastIntStack != 0)
	{
		gpLastIntStack = 0;

		__asm volatile 
			(
			"MOV.W SP, %0;"
			: : "r" (spStack)
			);

		END_EXCEPTION();
	}
	else if (spCurThread->Flags & THREAD_FLAG__PROBING)
	{
		CrReturnToSetProbe();
	}
	else
	{
		CrTerminateFaultyThread();
	}
}

void ExcReset()
{
	unsigned int *		pucDst = 0;
	unsigned int *		pucSrc = 0;

	/*
	 * Copy initialization value data from FLASH to data segment at SRAM
	 */
	for (pucDst = &_start_data, pucSrc = &_etext; pucDst < &_end_data; )
	{
		*pucDst++ = *pucSrc++;
	}

	/*
	 * Zero bss segment at SRAM
	 */
	for (pucDst = &_start_bss; pucDst < &_end_bss; ) {
		*pucDst++ = 0;
	}

	gpLastDbgStack		= 0;
	gpLastDbgFrameAddr	= 0;
	gpLastIntStack		= 0;
	guBusFaultAddress	= 0;
	guMemFaultAddress	= 0;

	guAppTID			= MAX_NUMB_OF_THREADS;

	ClkSetSystemClockToMaxSpeed();

	UaInitialize();
	UaWriteString("...\r\n");

	SCB_ENA_BUS_FAULT_EXCP(TRUE);
	SCB_ENA_MEM_FAULT_EXC(TRUE);
	SCB_ENA_USAGE_FAULT_EXCP(TRUE);

	UsbInitialize();
	ThdInitialize();
	EvtInitialize();
	ConInitialize();

	SCB_SET_SV_CALL_PRIO(2);
	SCB_SET_SYSTICK_PRIO(6);

	for (suVar = 0; suVar < NVIC_IRQ__MAXIMUM; suVar++)
	{
		NVIC_SET_IRQ_PRIO(suVar, 6);
	}

	NVIC_ENA_ALL_INTS();

	sfRes = CfgLoad();
	if (sfRes) {
		spCfg = CfgGet();
		if (spCfg->AutoRun) {
			sfRes = ThdCreate(
							0,
							spCfg->Control,
							FALSE,
							(THREAD_ENTRY_TYPE) APP_FLASH_RUN_ADDRESS,
							0,
							&guAppTID
							);
			if (!sfRes) {
				UaWriteString("Warning: fail to start auto run application!\r\n");
			}
		}
	}

	DBG_PRINTF("Park this thread.\r\n", 0);
	ThdParkThread();
}

UINT32_T CrGetLastFault()
{
	return suStat;
}

void CrSetFaultStatus(UINT32_T uStat)
{
	suStat = uStat;
	spCurThread = ThdGetCurrent();
}

BOOL CrIsAppRunning()
{
	PTHREAD		pTh;

	if (guAppTID >= MAX_NUMB_OF_THREADS) return FALSE;

	pTh = ThdGetFirstThread();
	pTh += guAppTID;

	if (pTh->Entry != (THREAD_ENTRY_TYPE) APP_FLASH_RUN_ADDRESS) return FALSE;
	
	if ((pTh->State >= THREAD_STATE__RUNNING) &&
		(pTh->State <= THREAD_STATE__SUSPENDED))
	{
		return TRUE;
	}

	return FALSE;
}

/*
 * Caller must be in privilege thread level or in exception level
 */
__attribute__((naked))
void CrTrapExecution(
				   UINT32_T TrapCode, 
				   UINT32_T Param1, 
				   UINT32_T Param2
				   )
{
	__asm volatile 
		(
		"CPSID f;"
		"CPSID i;"
		);

	__asm volatile 
		(
		"MOV.W SP, %0;"
		"SUB SP, #32;"
		"ADD R7, SP, #16;"
		"STR R0, [R7, #12];"
		"STR R1, [R7, #8];"
		"STR R2, [R7, #4];"
		: : "r" (SYSTEM_STACK_ADDRESS)
		);

	__asm volatile
		(
		"MOV.W %0, LR;"
		"LDR %1, [R7, #12];"
		"LDR %2, [R7, #8];"
		"LDR %3, [R7, #4];"
		: "=r" (suTpAddr), "=r" (suTpCode), "=r" (suTpParam1), "=r" (suTpParam2)
		);

	UsbShutdown();

	UaPrintf("\r\n[TRAP 0x%X] Address=0x%X, Param1=0x%X, Param2=0x%X\r\n",
		suTpCode, suTpAddr, suTpParam1, suTpParam2);

	suVar = 12800000;
	while (suVar-- > 0);

	UaPrintf("Reset the system...");
	SCB_RESET_SYSTEM();
}

/*
 * Reserved for experimental purpose
 */
//void ExcReset()
//{
//	unsigned int *		pucDst = 0;
//	unsigned int *		pucSrc = 0;
//	UINT32_T			uSpin;
//
//	for (pucDst = &_start_data, pucSrc = &_etext; pucDst < &_end_data; )
//	{
//		*pucDst++ = *pucSrc++;
//	}
//
//	for (pucDst = &_start_bss; pucDst < &_end_bss; ) {
//		*pucDst++ = 0;
//	}
//
//	DbgLED(2, 800000);
//
//	ClkSetSystemClockToMaxSpeed();
//
//	DbgLED(3, 800000);
//
//	UaInitialize(FALSE);
//	UaPrintf("...\r\n");
//
//	DbgLED(4, 800000);
//
//	while (TRUE)
//	{
//		//__asm volatile ("SVC #0");
//		uSpin = 8000000;
//		while (uSpin-- > 0);
//	}
//}