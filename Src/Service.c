/*
 * File    : Service.c
 * Remark  : Serve supervisor call requested by NanoOS API. The main purpose of supervisor call 
 *           in NanoOS actually is not for security or to serve a 'system service request' but 
 *           it rather for synchronization purpose, so the application can access 'system resource' 
 *           in multithread environment safer.
 *
 */

#include "Sys\CortexM\Scb.h"

#include "NanoOS.h"
#include "Thread.h"
#include "Service.h"
#include "ServiceDef.h"

extern PTHREAD ThdGetCurrent();

extern UINT32_T			guBusFaultAddress;
extern UINT32_T			guMemFaultAddress;

static PTHREAD			spCurThread;
static UINT32_PTR_T		spCurStack;
static UINT32_PTR_T		spCurFrame;
static UINT32_PTR_T		spStack;
static PSVC_PARAM		spParm;
static BOOL				sfNeedToSetStack;

static const SERVICE_CALL_TYPE sServices[SVC__MAXIMUM] = 
{
    /* 0    */ SvcZero,
    /* 1    */ SvcGetOSVersion,
    /* 2    */ SvcGetOSName,
	/* 3    */ SvcGetSystemClock,
	/* 4    */ SvcReserved,
	/* 5    */ SvcDebugCharOut,
	/* 6    */ SvcReserved,
	/* 7    */ SvcIsUsbSerReady,
	/* 8    */ SvcWriteUsbSer,
	/* 9    */ SvcReadUsbSer,
	/* 10	*/ SvcCancelReadUsbSer,
	/* 11	*/ SvcReserved,
    /* 12   */ SvcCreateThread,
    /* 13   */ SvcTerminateThread,
    /* 14   */ SvcExitThread,
    /* 15   */ SvcGetThreadReturnValue,
    /* 16   */ SvcGetThreadStatus,
	/* 17   */ SvcGetCurrentThreadId,
	/* 18   */ SvcSleep,
	/* 19   */ SvcSuspendThread,
	/* 20   */ SvcResumeThread,
	/* 21   */ SvcWaitForObject,
	/* 22   */ SvcSwitchToNextThread,
	/* 23   */ SvcReserved,
	/* 24   */ SvcReserved,
	/* 25   */ SvcReserved,
	/* 26   */ SvcReserved,
	/* 27   */ SvcCreateEvent,
	/* 28   */ SvcSetEvent,
	/* 29   */ SvcResetEvent,
	/* 30   */ SvcCloseEvent,
	/* 31   */ SvcReserved,
	/* 32   */ SvcReserved,
	/* 33   */ SvcSetTerminationHandler,
	/* 34   */ SvcGetFaultAddress,
	/* 35   */ SvcSetLastError,
	/* 36   */ SvcGetLastError,
	/* 37   */ SvcGetLastFault,
	/* 38   */ SvcSetIntHandler,
	/* 39   */ SvcReserved,
	/* 40   */ SvcReserved,
	/* 41   */ SvcLoadConfig,
	/* 42   */ SvcSaveConfig,
	/* 43   */ SvcReserved,
	/* 44   */ SvcReserved,
	/* 45   */ SvcUnlockFlash,
	/* 46   */ SvcLockFlash,
	/* 47   */ SvcEraseFlash,
	/* 48   */ SvcProgramFlash,
	/* 49   */ SvcReserved,
	/* 50   */ SvcReserved,
	/* 51	*/ SvcAcquireSpinLock,
	/* 52	*/ SvcReleaseSpinLock,
	/* 53	*/ SvcReserved,
	/* 54	*/ SvcSystemReset,
	/* 55	*/ SvcReserved,
	/* 56	*/ SvcSetProbe,
	/* 57	*/ SvcResetProbe,
	/* 58	*/ SvcReserved
};

/**** Begin services ***/
void SvcZero()
{
	DBG_PRINTF("%s: Index=0x%X\r\n", __FUNCTION__, spParm->Index);
	spParm->ReturnValue = TRUE;
}

void SvcGetOSVersion()
{
	if (spParm->Params[0] == 0) 
	{
		spCurThread->LastError = ERR__INVALID_RETURN_VALUE_POINTER;
		spParm->ReturnValue = FALSE;
		return;
	}

	*((UINT32_PTR_T) spParm->Params[0]) = PROG_VER;
	spParm->ReturnValue = TRUE;
}

void SvcGetOSName()
{
	UINT8_PTR_T	pName = (UINT8_PTR_T) spParm->Params[0];
	UINT32_T	iParamLen = spParm->Params[1];
	UINT32_T	iLen;

	iLen = NStrLen(PROG_NAME_STR);
	if (iLen >= iParamLen) 
	{
		spCurThread->LastError = ERR__INSUFFICIENT_BUFFER;
		spParm->ReturnValue = FALSE;
		return;
	}

	NMemCopy(pName, (UINT8_PTR_T) PROG_NAME_STR, iLen);
	spParm->ReturnValue = TRUE;
}

void SvcGetSystemClock()
{
	if (spParm->Params[0] == 0) 
	{
		spCurThread->LastError = ERR__INVALID_RETURN_VALUE_POINTER;
		spParm->ReturnValue = FALSE;
		return;
	}

	*((UINT32_PTR_T) spParm->Params[0]) = ClkGetSystemClock();
	spParm->ReturnValue = TRUE;
}

void SvcReserved()
{
	DBG_PRINTF("%s: Index=%d\r\n", __FUNCTION__, spParm->Index);
}

void SvcDebugCharOut()
{
	UaWriteChar((UINT8_T) spParm->Params[0]);
	__asm volatile ("MOV.W R1, #1;");	// Return value
}

void SvcIsUsbSerReady()
{
	spParm->ReturnValue = UsbIsReady();
}

void SvcWriteUsbSer()
{
	spParm->ReturnValue = 
		UsbSend((UINT8_PTR_T) spParm->Params[0], spParm->Params[1], TRUE);
}

void SvcCompleteReadUsbSer(UINT32_PTR_T pFirstParam)
{
	UsbReceive((UINT8_PTR_T) pFirstParam[0], pFirstParam[1], (UINT32_PTR_T) pFirstParam[2]);
	__asm volatile ("MOV.W R1, R0");
}

void SvcReadUsbSer()
{
	BOOL	fRes = FALSE;

	fRes = UsbRequestReceive();
	if (!fRes) {
		spParm->ReturnValue = FALSE;
		return;
	}

	spParm->Index = (UINT32_T) &spParm->Params[0];
	spParm->LR = spParm->PC;
	spParm->LR++;
	spParm->PC = (UINT32_T) SvcCompleteReadUsbSer;
}

void SvcCancelReadUsbSer()
{
	UsbCancelReceive();
	spParm->ReturnValue = TRUE;
}

void SvcCreateThread()
{
	BOOL fRes = FALSE;

	fRes = ThdCreate(
					0, 
					(UINT8_T) spParm->Params[0],
					(UINT8_T) spParm->Params[1],
					(THREAD_ENTRY_TYPE) spParm->Params[2], 
					(void *) spParm->Params[3], 
					(UINT32_PTR_T) spParm->Params[4]
					);
	
	spParm->ReturnValue = fRes;
}

void SvcTerminateThread()
{
	spParm->ReturnValue = 
		ThdTerminate(spParm->Params[0]);
}

void SvcExitThread()
{
	spParm->PC = (UINT32_T) ThdExit;
}

void SvcGetThreadReturnValue()
{
	spParm->ReturnValue = 
		ThdGetReturnValue(spParm->Params[0], (int *) spParm->Params[1]);
}

void SvcGetThreadStatus()
{
	BOOL		fRes = FALSE;
	UINT8_T		uState;

	if (spParm->Params[1] == 0) {
		spCurThread->LastError = ERR__INVALID_PARAMETER;
		spParm->ReturnValue = FALSE;
		return;
	}

	fRes = ThdGetState(spParm->Params[0], &uState, 0);
	if (!fRes) {
		spParm->ReturnValue = fRes;
		return;
	}

	switch (uState)
	{
	case THREAD_STATE__TERMINATED:
		*((UINT8_PTR_T) spParm->Params[1]) = THREAD_STATUS__TERMINATED;
		break;

	case THREAD_STATE__EXIT:
		*((UINT8_PTR_T) spParm->Params[1]) = THREAD_STATUS__EXIT;
		break;

	case THREAD_STATE__SUSPENDED:
		*((UINT8_PTR_T) spParm->Params[1]) = THREAD_STATUS__SUSPENDED;
		break;

	case THREAD_STATE__WAIT_FOR_OBJECT:
	case THREAD_STATE__SLEEP:
		*((UINT8_PTR_T) spParm->Params[1]) = THREAD_STATUS__SLEEP;
		break;

	default:
		*((UINT8_PTR_T) spParm->Params[1]) = THREAD_STATUS__ACTIVE;
		break;
	}

	spParm->ReturnValue = fRes;
}

void SvcGetCurrentThreadId()
{
	spParm->ReturnValue = 
		ThdGetCurrentId((UINT32_PTR_T) spParm->Params[0]);
}

void SvcSleep()
{
	BOOL	fRes = FALSE;
	
	fRes = ThdWait(
				THREAD_WAIT_TYPE__SLEEP,
				0,
				0,
				spParm->Params[0]
				);
	if (!fRes) {
		spParm->ReturnValue = FALSE;
		return;
	}

	spCurThread->SleepPC = spParm->PC;
	spCurThread->LastStackPtr = spCurStack;
	spParm->ReturnValue = TRUE;

	while (SCB_IS_PEND_SYSTICK() == FALSE);
}

void SvcSuspendThread()
{
	BOOL	fRes = FALSE;

	fRes = ThdSuspend(spParm->Params[0]);
	if (!fRes) {
		spParm->ReturnValue = FALSE;
		return;
	}

	spParm->ReturnValue = TRUE;
	
	if (spCurThread->Id == spParm->Params[0]) {
		spCurThread->SleepPC = spParm->PC;
		spCurThread->LastStackPtr = spCurStack;
		while (SCB_IS_PEND_SYSTICK() == FALSE);
	}
}

void SvcResumeThread()
{
	spParm->ReturnValue = ThdResume(spParm->Params[0]);
}

void SvcWaitForObject()
{
	BOOL	fRes = FALSE;
		
	fRes = ThdWait(
				THREAD_WAIT_TYPE__OBJECT,
				(UINT8_T) spParm->Params[0],
				spParm->Params[1],
				spParm->Params[2]
				);
	if (!fRes) {
		spParm->ReturnValue = FALSE;
		return;
	}

	spParm->ReturnValue = TRUE;
	
	spCurThread->SleepPC = spParm->PC;
	spCurThread->LastStackPtr = spCurStack;
	while (SCB_IS_PEND_SYSTICK() == FALSE);
}

void SvcSwitchToNextThread()
{
	while (SCB_IS_PEND_SYSTICK() == FALSE);
	spParm->ReturnValue = TRUE;
}

void SvcCreateEvent()
{
	spParm->ReturnValue = 
		EvtCreate((UINT32_PTR_T) spParm->Params[0], 0);
}

void SvcSetEvent()
{
	spParm->ReturnValue = EvtSet(spParm->Params[0], 0);
}

void SvcResetEvent()
{
	spParm->ReturnValue = EvtReset(spParm->Params[0], 0);
}

void SvcCloseEvent()
{
	spParm->ReturnValue = EvtClose(spParm->Params[0], 0);
}

void SvcSetTerminationHandler()
{
	spParm->ReturnValue = 
		ThdSetTerminationHandler((TERMINATION_HANDLER_TYPE) spParm->Params[0]);
}

void SvcGetFaultAddress()
{
	spParm->ReturnValue = FALSE;

	if (spParm->Params[0] == 0) 
		return;

	if (spParm->Params[0] & SCB_BUS_FAULT__VALID_FAULT_ADDRESS) {
		if (spParm->Params[1]) {
			*((UINT32_PTR_T) spParm->Params[1]) = guBusFaultAddress;
			spParm->ReturnValue = TRUE;
		}
	} else {
		if (spParm->Params[1]) {
			*((UINT32_PTR_T) spParm->Params[1]) = 0;
		}
	}

	if (spParm->Params[0] & SCB_MEM_FAULT__VALID_FAULT_ADDRESS) {
		if (spParm->Params[2]) {
			*((UINT32_PTR_T) spParm->Params[2]) = guMemFaultAddress;
			spParm->ReturnValue = TRUE;
		}
	} else {
		if (spParm->Params[2]) {
			*((UINT32_PTR_T) spParm->Params[2]) = 0;
		}
	}
}

void SvcGetLastFault()
{
	spParm->ReturnValue = CrGetLastFault();
}

void SvcSetLastError()
{
	spCurThread->LastError = spParm->Params[0];
	spParm->ReturnValue = TRUE;
}

void SvcGetLastError()
{
	spParm->ReturnValue = spCurThread->LastError;
}

void SvcSetIntHandler()
{
	spParm->ReturnValue =
		ExcSetInterrupt((INTERRUPT_HANDLER_TYPE) spParm->Params[0]);
}

/*
void SvcSetUsbIntHandler()
{
	spParm->ReturnValue =
		ExcSetUsbInterrupt((INTERRUPT_HANDLER_TYPE) spParm->Params[0]);
}
*/

void SvcLoadConfig()
{
	PCONFIG		pCfg = 0;
	UINT8_PTR_T	pDst = (UINT8_PTR_T) spParm->Params[0];

	if (spParm->Params[0] == 0) {
		spCurThread->LastError = ERR__INVALID_PARAMETER;
		spParm->ReturnValue = FALSE;
		return;
	}

	CfgLoad();
	pCfg = CfgGet();
	if (!pCfg) {
		spParm->ReturnValue = FALSE;
		return;
	}

	NMemCopy(pDst, (UINT8_PTR_T) pCfg, sizeof(CONFIG));

	spParm->ReturnValue = TRUE;
}

void SvcSaveConfig()
{
	UINT8_PTR_T	pSrc = (UINT8_PTR_T) spParm->Params[0];
	PCONFIG		pCfg = 0;

	if (spParm->Params[0] == 0) {
		spCurThread->LastError = ERR__INVALID_PARAMETER;
		spParm->ReturnValue = FALSE;
		return;
	}

	pCfg = CfgGet();
	if (!pCfg) {
		spParm->ReturnValue = FALSE;
		return;
	}

	NMemCopy((UINT8_PTR_T) pCfg, pSrc, sizeof(CONFIG));

	spParm->ReturnValue = CfgSave();
}

void SvcUnlockFlash()
{
	spParm->ReturnValue = FlsUnlock();
}

void SvcLockFlash()
{
	spParm->ReturnValue = FlsLock();
}

void SvcEraseFlash()
{
	UINT32_PTR_T	uiFlashAddr = 0;
	BOOL			fRes = FALSE;

	fRes = FlsGetAddressFromPageNo(spParm->Params[0], &uiFlashAddr);
	if (!fRes) {
		spParm->ReturnValue = FALSE;
		return;
	}
	if ((((UINT32_T) uiFlashAddr) >= FLASH_WRITE_BASE_ADDRESS) &&
		(((UINT32_T) uiFlashAddr) <= (APP_FLASH_WRITE_ADDRESS - 1)))
	{
		spCurThread->LastError = ERR__INVALID_FLASH_PAGE_NO;
		spParm->ReturnValue = FALSE;
		return;
	}

	spParm->ReturnValue = FlsErase(spParm->Params[0], spParm->Params[1]);
}

void SvcProgramFlash()
{
	spParm->ReturnValue =
		FlsProgram(
				(UINT16_PTR_T) spParm->Params[0], 
				(UINT16_PTR_T) spParm->Params[1], 
				spParm->Params[2]
				);
}

void SvcAcquireSpinLock()
{
	if (*((UINT32_PTR_T) spParm->Params[0]) == 0)
	{
		*((UINT32_PTR_T) spParm->Params[0]) = 1;
		spParm->ReturnValue = TRUE;
		return;
	}
	spParm->ReturnValue = FALSE;
}

void SvcReleaseSpinLock()
{
	*((UINT32_PTR_T) spParm->Params[0]) = 0;
	spParm->ReturnValue = TRUE;
	while (SCB_IS_PEND_SYSTICK() == FALSE);
}

void SvcSystemReset()
{
	SCB_RESET_SYSTEM();
}

/*
 * Safer SetProbe/ResetProbe but require more space and slower
 *
void SvcSetProbe()
{
	BOOL	fRes = FALSE;

	if (spParm->Params[0] == 0) {
		ThdSetLastError(ERR__INVALID_PARAMETER);
		spParm->ReturnValue = fRes;
		return;
	}

	if (spCurThread->Flags & THREAD_FLAG__PROBING)
	{
		ThdSetLastError(ERR__ALREADY_PROBING);
		spParm->ReturnValue = FALSE;
		return;
	}

	fRes = ThdSaveCurrentThread(spCurStack);
	if (!fRes) {
		spParm->ReturnValue = fRes;
		return;
	}

	*((UINT32_PTR_T) spParm->Params[0]) = PROBE_STATUS__ORIGIN;
	spCurThread->Flags |= THREAD_FLAG__PROBING;

	spParm->ReturnValue = TRUE;
}

void SvcResetProbe()
{
	BOOL		fRes = FALSE;
	PSVC_PARAM	pParm;

	if ((spCurThread->Flags & THREAD_FLAG__PROBING) == 0)
	{
		ThdSetLastError(ERR__PROBE_NOT_SET);
		spParm->ReturnValue = FALSE;
		return;
	}

	if (spParm->Params[0] == FALSE) {
		fRes = ThdReleaseCurrentSaveTID();
		spCurThread->Flags &= ~(THREAD_FLAG__PROBING);
		spParm->ReturnValue = fRes;
		return;
	}

	fRes = ThdRestoreCurrentThread();
	if (!fRes) {
		spCurThread->Flags &= ~(THREAD_FLAG__PROBING);
		spParm->ReturnValue = FALSE;
		return;
	}

	pParm = (PSVC_PARAM) spCurThread->LastStackPointer;
	pParm->ReturnValue = TRUE;

	*((UINT32_PTR_T) pParm->Params[0]) = PROBE_STATUS__RETURN;
	spCurThread->Flags &= ~(THREAD_FLAG__PROBING);

	sfNeedToSetStack = TRUE;
}
*/

void SvcSetProbe()
{
	if (spCurThread->Flags & THREAD_FLAG__PROBING)
	{
		ThdSetLastError(ERR__ALREADY_PROBING);
		spParm->ReturnValue = PROBE_STATUS__ERROR;
		return;
	}

	spCurThread->ProbeStackPtr		= spCurStack;
	spCurThread->ProbeFrameAddress	= *spCurStack;
	spCurThread->ProbeLR			= spParm->LR;
	spCurThread->ProbePC			= spParm->PC;
	
	spCurThread->Flags	|= THREAD_FLAG__PROBING;

	spParm->ReturnValue = PROBE_STATUS__ORIGIN;
}

void SvcResetProbe()
{
	PINITIAL_THREAD_STACK	pInitStack = 0;

	if ((spCurThread->Flags & THREAD_FLAG__PROBING) == 0)
	{
		ThdSetLastError(ERR__PROBE_NOT_SET);
		spParm->ReturnValue = PROBE_STATUS__ERROR;
		return;
	}

	if (spCurStack > spCurThread->ProbeStackPtr)
	{
		ThdSetLastError(ERR__INVALID_PROBE_STACK);
		spParm->ReturnValue = PROBE_STATUS__ERROR;
		return;
	}

	if (spParm->Params[0] == FALSE) {
		spCurThread->Flags &= ~(THREAD_FLAG__PROBING);
		spParm->ReturnValue = PROBE_STATUS__NO_RETURN;
		return;
	}

	pInitStack = (PINITIAL_THREAD_STACK) spCurThread->ProbeStackPtr;
	pInitStack->FrameAddress	= spCurThread->ProbeFrameAddress;
	pInitStack->ExcReturn		= EXC_RETURN;
	pInitStack->ExcFrame.LR		= spCurThread->ProbeLR;
	pInitStack->ExcFrame.PC		= spCurThread->ProbePC;
	pInitStack->ExcFrame.R1		= PROBE_STATUS__RETURN;
	pInitStack->ExcFrame.PSR	= spParm->PSR;

	spCurThread->LastStackPtr	= spCurThread->ProbeStackPtr;
	spCurThread->Flags			&= ~(THREAD_FLAG__PROBING);

	sfNeedToSetStack = TRUE;
}
/**** End of services ***/

__attribute__((naked))
void ExcSupervisorCall()
{
	BEGIN_EXCEPTION();

	GET_STACK_POINTER(spCurStack);
	GET_FRAME_ADDRESS(spCurFrame);

	spCurThread = ThdGetCurrent();
	if ((spCurStack <= spCurThread->LimitStackPtr) ||
		(spCurFrame <= spCurThread->LimitStackPtr))
	{
		while (SCB_IS_PEND_SYSTICK() == FALSE);
		END_EXCEPTION();
	}

	spStack = spCurStack;

	spStack++;
	TRAP_EXEC(
		(*spStack != EXC_RETURN),
		TRAP__INVALID_SERVICE_STACK,
		(UINT32_T) spStack,
		*spStack
		);

	spStack++;
	spParm = (PSVC_PARAM) spStack;

	TRAP_EXEC(
		(spParm->Index >= SVC__MAXIMUM),
		TRAP__INVALID_SERVICE_INDEX,
		spParm->Index, 
		0
		);

	sfNeedToSetStack = FALSE;
	
	(* sServices[spParm->Index])();

	if (sfNeedToSetStack)
	{
		__asm volatile 
			(
			"MOV.W SP, %0;"
			: : 
				"r" (spCurThread->LastStackPtr)
			);
	}

	END_EXCEPTION();
}
