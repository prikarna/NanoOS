/*
 * File    : Thread.c
 * Remark  : Thread system. NanoOS thread has no priority. They will be treated equally in round-robin manner. 
 *           This thread model has no concept of 'idle thread', a thread that reach the end of its execution 
 *           will be 'parked'. This thread model, for now, can't accept external stack, all thread use internal 
 *           stack maintained by this thread system. A thread can run in two mode: privileged 
 *           (THREAD_CONTROL__PRIVILEGED) and unprivileged (THREAD_CONTROL__UNPRIVILEGED). In privileged mode
 *           thread can access Cotex-M peripherals (SCB, NVIC etc.) and in unprivileged mode thread has no access
 *           to that peripherals.
 *           Exception system tick handler is used to do thread switching and to monitor the stack.
 *           Timer2 interrupt is used to manage counted sleeping thread.
 *
 */

#include "Sys\CortexM\SysTick.h"
#include "Sys\CortexM\Scb.h"
#include "Sys\Stm\Rcc.h"
#include "Sys\Stm\Timer.h"

#include "NanoOS.h"
#include "Thread.h"
#include "Service.h"
#include "Event.h"

extern UINT32_PTR_T		gpLastDbgStack;
extern UINT32_PTR_T		gpLastDbgFrameAddr;
extern UINT32_PTR_T		gpLastIntStack;

extern void CrReturnToSetProbe();
extern void CrTerminateFaultyThread();
extern void CrSetFaultStatus(UINT32_T uStat);

static THREAD_STACK		sStacks[MAX_NUMB_OF_THREADS];
static THREAD			sThreads[MAX_NUMB_OF_THREADS];

/*
 * Variables
 */
static PTHREAD			spCurThread;
static PTHREAD			spNextThread;
static UINT32_PTR_T		spCurStack;
static UINT32_PTR_T		spCurFrame;
static UINT32_T			suVar;
static PINITIAL_THREAD_STACK	spInitStack;

static PTHREAD		spTh;
static UINT16_T		suEvtState;
static UINT8_T		suThState;
static BOOL			sfRes;

static void CallThread(PTHREAD pThread)
{
	pThread->ReturnValue = pThread->Entry(pThread->Parameter);
	pThread->State = THREAD_STATE__EXIT;
}

__attribute__((naked))
void ThdParkThread()
{
	__asm volatile 
		(
		"MOV.W SP, %0;"
		"SUB SP, #8;"
		"MOV.W R7, SP;"
		"NOP;"
		: : "r" (SYSTEM_STACK_ADDRESS)
		);

	while (TRUE);
}

static void GetNextAndSaveCurrentThread()
{
	spNextThread = spCurThread;
	do {
		if (spNextThread->State == THREAD_STATE__WAITING)
		{
			break;
		}
		spNextThread = spNextThread->BLink;
	} while (spNextThread != spCurThread);

	switch (spCurThread->State)
	{
	case THREAD_STATE__RUNNING:
		TRAP_EXEC(
				(spCurStack > spCurThread->StartStackPtr),
				TRAP__CUR_STACK_PTR_GT_START_STACK_PTR,
				(UINT32_T) spCurStack,
				(UINT32_T) spCurThread->StartStackPtr
				);

		if (spNextThread != spCurThread) {
			spCurThread->LastStackPtr = spCurStack;
			spCurThread->State = THREAD_STATE__WAITING;
		}
		break;

	case THREAD_STATE__SLEEP:
	case THREAD_STATE__WAIT_FOR_OBJECT:
	case THREAD_STATE__SUSPENDED:
		if (spNextThread == spCurThread) {
			spInitStack = (PINITIAL_THREAD_STACK) spCurThread->LastStackPtr;
			spInitStack->ExcFrame.PC = (UINT32_T) ThdParkThread;
		}
		break;

	default:
		break;
	}
}

__attribute__((naked))
void ExcSystemTick()
{
	BEGIN_EXCEPTION();

	GET_STACK_POINTER(spCurStack);
	GET_FRAME_ADDRESS(spCurFrame);

	if (spCurThread->State == THREAD_STATE__RUNNING)
	{
		if ((spCurStack <= spCurThread->LimitStackPtr) ||
			(spCurFrame <= spCurThread->LimitStackPtr))
		{
			SET_TO_SYSTEM_STACK();

			gpLastDbgStack = spCurStack;
			gpLastDbgFrameAddr = spCurFrame;

			CrSetFaultStatus(TERM__STACK_REACH_ITS_LIMIT_FAULT);
			if (spCurThread->Flags & THREAD_FLAG__PROBING) {
				CrReturnToSetProbe();
			} else {
				CrTerminateFaultyThread();
			}
		}
	}

	GetNextAndSaveCurrentThread();
	if (spNextThread == spCurThread) 
	{
		if (spCurThread->State != THREAD_STATE__WAITING) 
		{
			END_EXCEPTION();
		}
	}

	spCurThread = spNextThread;
	spCurThread->State = THREAD_STATE__RUNNING;

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

void ThdInitialize()
{
	spNextThread = 0;
	spCurThread = &(sThreads[MAX_NUMB_OF_THREADS - 1]);
	for (suVar = 0; suVar < MAX_NUMB_OF_THREADS; suVar++)
	{
		sThreads[suVar].State = THREAD_STATE__EXIT;
		if (suVar == 0) {
			sThreads[suVar].BLink = &(sThreads[MAX_NUMB_OF_THREADS - 1]);
		} else {
			sThreads[suVar].BLink = &(sThreads[suVar-1]);
		}
	}

	RCC_ENA_APB1_CTRL_CLK(TRUE, RCC_APB1_CTRL__TIM2);

	TIM_SET_PRESCALER(TIM2, TIMER_PRESCALE);
	TIM_SET_AUTO_RELOAD(TIM2, TIMER_AUTO_RELOAD);
	TIM_ENA_OVER_OR_UNDERFLOW_UPDT_SRC(TIM2, TRUE);
	TIM_ENA_UPDATE_INTERRUPT(TIM2, TRUE);
	TIM_ENA_COUNTER(TIM2, TRUE);

	STK_SET_RELOAD(THREAD_TICK_COUNT__LEVEL_0);
	STK_ENABLE(STK_OPT__ENA_EXCEPTION);
}

UINT8_T ThdCreate(
				  UINT32_PTR_T pStartStack,
				  UINT8_T uiThreadControl,
				  UINT8_T fIsSuspended,
				  THREAD_ENTRY_TYPE ThreadEntry, 
				  void * pParameter, 
				  UINT32_PTR_T pThreadId
				  )
{
	UINT32_T				uiTID;
	PTHREAD					pThread;
	PINITIAL_THREAD_STACK	pInitStack;
	UINT32_T				uStackSize;
	UINT32_PTR_T			pStartStackAddr;
	PTHREAD_STACK			pThdStack;
	UINT8_T					fRes = FALSE;

	if (ThreadEntry == 0) {
		spCurThread->LastError = ERR__INVALID_THREAD_ENTRY;
		return FALSE;
	}

	pThread = &(sThreads[MAX_NUMB_OF_THREADS - 1]);
	uiTID = (MAX_NUMB_OF_THREADS - 1);
	for (;;) {
		if ((pThread->State == THREAD_STATE__EXIT) ||
			(pThread->State == THREAD_STATE__TERMINATED))
		{
			fRes = TRUE;
			pThread->State = THREAD_STATE__QUEUEING;
			break;
		}
		if (pThread == &(sThreads[0])) {
			break;
		}
		pThread--;
		uiTID--;
	}

	if (fRes == FALSE) {
		spCurThread->LastError = ERR__NO_MORE_THREAD_SLOT;
		return FALSE;
	}

	if (pStartStack == 0) {
		pThdStack = &(sStacks[0]);
		pThdStack += uiTID;
		pStartStackAddr = &(pThdStack->Stack[THREAD_STACK_SIZE - 1]);
	} else {
		pStartStackAddr = pStartStack;
	}

	uStackSize = SIZEOF_INITIAL_STACK();
	pThread->StartStackPtr	= pStartStackAddr;
	pThread->LastStackPtr	= (pStartStackAddr - (uStackSize - 1));

	switch (uiThreadControl)
	{
	case THREAD_CONTROL__PRIVILEGED:
		pThread->Control = THREAD_CONTROL__PRIVILEGED;
		break;

	case THREAD_CONTROL__UNPRIVILEGED:
		pThread->Control = THREAD_CONTROL__UNPRIVILEGED;
		break;

	default:
		pThread->State = THREAD_STATE__EXIT;
		spCurThread->LastError = ERR__INVALID_THREAD_CONTROL;
		return FALSE;
	}

	pInitStack = (PINITIAL_THREAD_STACK) pThread->LastStackPtr;

	pInitStack->FrameAddress	= (UINT32_T) pStartStackAddr;
	pInitStack->ExcReturn		= EXC_RETURN;
	pInitStack->ExcFrame.R0		= (UINT32_T) pThread;
	pInitStack->ExcFrame.PC		= (UINT32_T) CallThread;
	pInitStack->ExcFrame.LR		= (UINT32_T) ThdParkThread;
	pInitStack->ExcFrame.PSR	= BITHEX_24;

	pThread->Counter				= 0;
	pThread->WaitObjectId			= 0;
	pThread->WaitObjectType			= 0;
	pThread->TermHandler			= 0;

	pThread->Entry					= ThreadEntry;
	pThread->Parameter				= pParameter;
	pThread->ReturnValue			= 0;

	pThread->SleepPC				= 0;
	pThread->LastError				= 0;
	pThread->Flags					= THREAD_FLAG__NONE;

	if (pThreadId) {
		*pThreadId					= uiTID;
	}

	pThread->Id						= uiTID;

	pThread->LimitStackPtr			= &(pThdStack->Stack[0]) + THREAD_STACK_LIMIT_DELTA;

	if (fIsSuspended) {
		pThread->State				= THREAD_STATE__SUSPENDED;
	} else {
		pThread->State				= THREAD_STATE__WAITING;
	}

	return TRUE;
}

UINT8_T ThdGetReturnValue(UINT32_T uThreadId, int * pThreadReturnValue)
{
	if (pThreadReturnValue == 0) {
		spCurThread->LastError = ERR__INVALID_PARAMETER;
		return FALSE;
	}

	if (uThreadId >= MAX_NUMB_OF_THREADS) {
		spCurThread->LastError = ERR__INVALID_THREAD_ID;
		return FALSE;
	}

	*pThreadReturnValue = sThreads[uThreadId].ReturnValue;

	return TRUE;
}

/*
UINT8_T ThdGetState(UINT32_T uThreadId, UINT8_PTR_T pThreadState)
{
	PTHREAD		pTh;

	if (pThreadState == 0) {
		spCurThread->LastError = ERR__INVALID_RETURN_VALUE_POINTER;
		return FALSE;
	}

	if (uThreadId >= MAX_NUMB_OF_THREADS) {
		spCurThread->LastError = ERR__INVALID_THREAD_ID;
		return FALSE;
	}

	pTh = &sThreads[0];
	pTh += uThreadId;

	*pThreadState = pTh->State;

	return TRUE;
}
*/

UINT8_T ThdGetState(UINT32_T uThreadId, UINT8_PTR_T pThreadState, PTHREAD pRequestingThread)
{
	if (pThreadState == 0) {
		if (pRequestingThread) {
			pRequestingThread->LastError = ERR__INVALID_PARAMETER;
		} else {
			spCurThread->LastError = ERR__INVALID_PARAMETER;
		}
		return FALSE;
	}

	if (uThreadId >= MAX_NUMB_OF_THREADS) {
		if (pRequestingThread) {
			pRequestingThread->LastError = ERR__INVALID_THREAD_ID;
		} else {
			spCurThread->LastError = ERR__INVALID_THREAD_ID;
		}
		return FALSE;
	}

	*pThreadState = sThreads[uThreadId].State;

	return TRUE;
}

/*
UINT8_T ThdGetCurrentState(UINT8_PTR_T puiState)
{
	if (!puiState) {
		spCurThread->LastError = ERR__INVALID_RETURN_VALUE_POINTER;
		return FALSE;
	}

	*puiState = spCurThread->State;

	return TRUE;
}
*/

UINT8_T ThdTerminate(UINT32_T uThreadId)
{
	PTHREAD		pTh;

	if ((uThreadId >= MAX_NUMB_OF_THREADS) ||
		(uThreadId == spCurThread->Id))
	{
		spCurThread->LastError = ERR__INVALID_THREAD_ID;
		return FALSE;
	}

	pTh = &(sThreads[0]);
	pTh += uThreadId;

	if (pTh->State == THREAD_STATE__TERMINATED) {
		return TRUE;
	}

	if (pTh->State == THREAD_STATE__EXIT) {
		spCurThread->LastError = ERR__INVALID_THREAD_STATE;
		return FALSE;
	}

	if ((pTh->Flags & THREAD_FLAG__IO_TXRX) == THREAD_FLAG__IO_TXRX)
	{
		pTh->Flags &= ~(THREAD_FLAG__IO_TXRX);
	}

	if (pTh->TermHandler == 0) {
		pTh->State = THREAD_STATE__TERMINATED;
	} else {
		if ((pTh->State >= THREAD_STATE__SLEEP) &&
			(pTh->State <= THREAD_STATE__SUSPENDED))
		{
			pTh->SleepPC			= (UINT32_T) pTh->TermHandler;
			pTh->State				= THREAD_STATE__WAITING;
		}

		pTh->Flags					|= THREAD_FLAG__TERMINATING;
		pTh->Counter				= TERMINATING_COUNT;

		spInitStack					= (PINITIAL_THREAD_STACK) pTh->LastStackPtr;
		spInitStack->ExcFrame.PC	= (UINT32_T) pTh->TermHandler;
		spInitStack->ExcFrame.LR	= (UINT32_T) ThdExit;
		spInitStack->ExcFrame.R0	= TERM__USER_REQUEST;

		pTh->TermHandler			= 0;
	}

	return TRUE;
}

UINT8_T ThdGetCurrentId(UINT32_PTR_T pRetTID)
{
	if (!pRetTID) {
		spCurThread->LastError = ERR__INVALID_PARAMETER;
		return FALSE;
	}

	*pRetTID = spCurThread->Id;
	
	return TRUE;
}

/*
UINT8_T ThdGetControlType(UINT32_T uThreadId, UINT8_PTR_T pControlType)
{
	PTHREAD		pTh;

	if (uThreadId >= MAX_NUMB_OF_THREADS) {
		spCurThread->LastError = ERR__INVALID_THREAD_ID;
		return FALSE;
	}

	if (pControlType == 0) {
		spCurThread->LastError = ERR__INVALID_RETURN_VALUE_POINTER;
		return FALSE;
	}

	pTh = &(sThreads[0]);
	pTh += uThreadId;

	if ((pTh->State == THREAD_STATE__RUNNING) ||
		(pTh->State == THREAD_STATE__WAITING))
	{
		*pControlType = pTh->Control;
		return TRUE;
	}

	spCurThread->LastError = ERR__INVALID_THREAD_STATE;

	return FALSE;
}
*/

void ThdExit()
{
	if (spCurThread->Flags & THREAD_FLAG__TERMINATING) {
		spCurThread->Flags &= ~(THREAD_FLAG__TERMINATING);
		spCurThread->State = THREAD_STATE__TERMINATED;
	} else {
		spCurThread->State = THREAD_STATE__EXIT;
	}

	ThdParkThread();
}

BOOL ThdWait(
			 UINT8_T uiWaitType, 
			 UINT8_T uiObjectType, 
			 UINT32_T uiObjectId, 
			 UINT32_T uiMilliSec
			 )
{
	BOOL		fRes = FALSE;
	UINT16_T	uEvtState = 0;
	UINT8_T		uThdState = 0;

	if ((uiMilliSec < THREAD__MIN_MSEC_SLEEP) ||
		(uiMilliSec > THREAD__INFINITE_WAIT))
	{
		spCurThread->LastError = ERR__INVALID_MILISEC_VALUE;
		return FALSE;
	}

	switch (uiWaitType)
	{
	case THREAD_WAIT_TYPE__SLEEP:
		if (uiMilliSec >= THREAD__INFINITE_WAIT) {
			spCurThread->LastError = ERR__INVALID_MILISEC_VALUE;
			return FALSE;
		}
		spCurThread->Counter = uiMilliSec;
		spCurThread->State = THREAD_STATE__SLEEP;
		break;

	case THREAD_WAIT_TYPE__OBJECT:
		switch (uiObjectType)
		{
		case THREAD_WAIT_OBJ__EVENT:
			fRes = EvtGetState(uiObjectId, &uEvtState, 0);
			if (!fRes) {
				return FALSE;
			}
			if ((uEvtState & EVT__SET_BIT) == EVT__SET_BIT) {
				spCurThread->LastError = ERR__EVENT_ALREADY_BEEN_SET;
				return FALSE;
			}

			spCurThread->WaitObjectType = THREAD_WAIT_OBJ__EVENT;
			break;

		case THREAD_WAIT_OBJ__THREAD:
			fRes = ThdGetState(uiObjectId, &uThdState, 0);
			if (!fRes) 
				return FALSE;
			
			if ((uThdState == THREAD_STATE__EXIT) ||
				(uThdState == THREAD_STATE__TERMINATED))
			{
				spCurThread->LastError = ERR__INVALID_THREAD_STATE;
				return FALSE;
			}

			spCurThread->WaitObjectType = THREAD_WAIT_OBJ__THREAD;
			break;

		default:
			spCurThread->LastError = ERR__INVALID_THREAD_WAIT_OBJ_TYPE;
			return FALSE;
		}

		spCurThread->Counter = uiMilliSec;

		spCurThread->WaitObjectId = uiObjectId;
		spCurThread->State = THREAD_STATE__WAIT_FOR_OBJECT;
		break;

	case THREAD_WAIT_TYPE__INTERRUPT:
		spCurThread->State = THREAD_STATE__SLEEP;
		spCurThread->Counter = THREAD__INFINITE_WAIT;
		break;

	default:
		spCurThread->LastError = ERR__INVALID_THREAD_WAIT_TYPE;
		return FALSE;
	}

	return TRUE;
}

/*
BOOL ThdGetCurrentControlType(UINT8_PTR_T puiControlType)
{
	if (!puiControlType) {
		spCurThread->LastError = ERR__INVALID_RETURN_VALUE_POINTER;
		return FALSE;
	}

	*puiControlType = spCurThread->Control;

	return TRUE;
}
*/

BOOL ThdSuspend(UINT32_T uiThreadId)
{
	if (uiThreadId >= MAX_NUMB_OF_THREADS) {
		spCurThread->LastError = ERR__INVALID_THREAD_ID;
		return FALSE;
	}

	if ((sThreads[uiThreadId].State < THREAD_STATE__RUNNING) ||
		(sThreads[uiThreadId].State > THREAD_STATE__WAITING))
	{
		spCurThread->LastError = ERR__INVALID_THREAD_STATE;
		return FALSE;
	}

	sThreads[uiThreadId].State = THREAD_STATE__SUSPENDED;

	return TRUE;
}

BOOL ThdResume(UINT32_T uiThreadId)
{
	if ((uiThreadId >= MAX_NUMB_OF_THREADS) ||
		(uiThreadId == spCurThread->Id))
	{
		spCurThread->LastError = ERR__INVALID_THREAD_ID;
		return FALSE;
	}

	if (sThreads[uiThreadId].State != THREAD_STATE__SUSPENDED) {
		spCurThread->LastError = ERR__INVALID_THREAD_STATE;
		return FALSE;
	}

	sThreads[uiThreadId].State = THREAD_STATE__WAITING;

	return TRUE;
}

BOOL ThdSetTerminationHandler(TERMINATION_HANDLER_TYPE TermHandler)
{
	if (spCurThread->Flags & THREAD_FLAG__TERMINATING)
	{
		spCurThread->LastError = ERR__OPERATION_NOT_ALLOWED;
		return FALSE;
	}

	spCurThread->TermHandler = TermHandler;
	return TRUE;
}

/*
TERMINATION_HANDLER_TYPE ThdGetCurrentTerminationHandler()
{
	return spCurThread->TermHandler;
}
*/

PTHREAD ThdGetCurrent()
{
	return spCurThread;
}

PTHREAD ThdGetFirstThread()
{
	return &sThreads[0];
}

PTHREAD_STACK ThdGetFirstStack()
{
	return &sStacks[0];
}

void ThdSetLastError(UINT32_T uiError)
{
	spCurThread->LastError = uiError;
}

UINT32_T ThdGetLastError()
{
	return spCurThread->LastError;
}

PTHREAD ThdGetWaitingInterrupt(UINT32_T uiIntNumb, UINT32_T uiFlags)
{
	PTHREAD pTh = &sThreads[MAX_NUMB_OF_THREADS - 1];

	do {
		if ((pTh->Flags & uiFlags) == uiFlags)
			break;

		pTh = pTh->BLink;
	} while (pTh != &sThreads[MAX_NUMB_OF_THREADS - 1]);

	if (pTh == &sThreads[MAX_NUMB_OF_THREADS - 1])
		return 0;
	
	return pTh;
}

void IntTimer2()
{
	spTh = &sThreads[0];
	for (suVar = 0; suVar < MAX_NUMB_OF_THREADS; suVar++)
	{
		if (spTh->Flags & THREAD_FLAG__TERMINATING) {
			if (spTh->Counter > 0) {
				spTh->Counter--;
			} else {
				spTh->State = THREAD_STATE__TERMINATED;
				spTh->Flags &= ~(THREAD_FLAG__TERMINATING);
			}
			spTh++;
			continue;
		}

		switch (spTh->State)
		{
		case THREAD_STATE__SLEEP:
			if (spTh->Counter != THREAD__INFINITE_WAIT)
			{
				if (spTh->Counter > 0) 
				{
					spTh->Counter--;
				} 
				else 
				{
					// Completed
					spInitStack = (PINITIAL_THREAD_STACK) spTh->LastStackPtr;
					spInitStack->ExcFrame.PC = spTh->SleepPC;		// Restore PC
					spTh->State = THREAD_STATE__WAITING;			// Change thread state
					spTh->LastError = ERR__NONE;
				}
			}
			break;


		case THREAD_STATE__WAIT_FOR_OBJECT:
			if (spTh->WaitObjectType == THREAD_WAIT_OBJ__EVENT)
			{
				sfRes = EvtGetState(spTh->WaitObjectId, &suEvtState, spTh);
				if (sfRes) 
				{
					if (suEvtState & EVT__SET_BIT) 
					{
						// Completed
						spInitStack = (PINITIAL_THREAD_STACK) spTh->LastStackPtr;
						spInitStack->ExcFrame.PC = spTh->SleepPC;		// Restore PC
						spTh->State = THREAD_STATE__WAITING;			// Change thread state
						spTh->LastError = ERR__NONE;
					} 
					else 
					{
						if (spTh->Counter != THREAD__INFINITE_WAIT)
						{
							if (spTh->Counter > 0) {
								spTh->Counter--;
							} else {
								// Time out
								spInitStack = (PINITIAL_THREAD_STACK) spTh->LastStackPtr;
								spInitStack->ExcFrame.PC = spTh->SleepPC;		// Restore PC
								spInitStack->ExcFrame.R1 = FALSE;				// Set return value
								spTh->State = THREAD_STATE__WAITING;			// Change thread state
								spTh->LastError = ERR__WAIT_TIMEOUT;
							}
						}
					}
				}
				else
				{
					// If error while get the state of the event
					spInitStack = (PINITIAL_THREAD_STACK) spTh->LastStackPtr;
					spInitStack->ExcFrame.PC = spTh->SleepPC;		// Restore PC
					spInitStack->ExcFrame.R1 = FALSE;				// Set return value
					spTh->State = THREAD_STATE__WAITING;			// Change the state
				}
			}
			else if (spTh->WaitObjectType == THREAD_WAIT_OBJ__THREAD)
			{
				sfRes = ThdGetState(spTh->WaitObjectId, &suThState, spTh);
				if (sfRes) {
					if ((suThState == THREAD_STATE__EXIT) ||
						(suThState == THREAD_STATE__TERMINATED))
					{
						// Completed
						spInitStack = (PINITIAL_THREAD_STACK) spTh->LastStackPtr;
						spInitStack->ExcFrame.PC = spTh->SleepPC;			// Restore PC
						spTh->State = THREAD_STATE__WAITING;				// Change the state
						spTh->LastError = ERR__NONE;
					}
					else
					{
						if (spTh->Counter != THREAD__INFINITE_WAIT) 
						{
							if (spTh->Counter > 0) {
								spTh->Counter--;
							} else {
								// Time out
								spInitStack = (PINITIAL_THREAD_STACK) spTh->LastStackPtr;
								spInitStack->ExcFrame.PC = spTh->SleepPC;		// Restore PC
								spInitStack->ExcFrame.R1 = FALSE;				// Set return valuse
								spTh->State = THREAD_STATE__WAITING;			// Change the state
								spTh->LastError = ERR__WAIT_TIMEOUT;
							}
						}
					}
				} else {
					// If error while get the state of the thread
					spInitStack = (PINITIAL_THREAD_STACK) spTh->LastStackPtr;
					spInitStack->ExcFrame.PC = spTh->SleepPC;		// Restore PC
					spInitStack->ExcFrame.R1 = FALSE;				// As return value
					spTh->State = THREAD_STATE__WAITING;			// Change the state
				}
			}
			else
			{
			}
			break;

		default:
			break;
		}

		spTh++;
	}

	TIM_CLR_UPDATE_INT_FLAG(TIM2);
}

/*
 * Require more space
 *
BOOL ThdSaveCurrentThread(UINT32_PTR_T pCurStack)
{
	UINT32_T				uiTID;
	PTHREAD					pThread;
	PTHREAD_STACK			pThdStack;
	UINT32_PTR_T			pDst, pSrc;

	if (pCurStack == 0) {
		spCurThread->LastError = ERR__INVALID_PARAMETER;
		return FALSE;
	}

	pThread = &(sThreads[0]);
	for (uiTID = 0; uiTID < MAX_NUMB_OF_THREADS; uiTID++) {
		if ((pThread->State == THREAD_STATE__EXIT) ||
			(pThread->State == THREAD_STATE__TERMINATED))
		{
			pThread->State = THREAD_STATE__QUEUEING;
			break;
		}
		pThread++;
	}

	if (uiTID == MAX_NUMB_OF_THREADS) {
		spCurThread->LastError = ERR__NO_MORE_THREAD_SLOT;
		return FALSE;
	}

	pThread->Control				= spCurThread->Control;
	pThread->Counter				= spCurThread->Counter;
	pThread->Entry					= spCurThread->Entry;
	pThread->LastError				= spCurThread->LastError;
	pThread->Parameter				= spCurThread->Parameter;
	pThread->ReturnValue			= spCurThread->ReturnValue;
	pThread->SavedPC				= spCurThread->SavedPC;
	pThread->StartStackPointer		= spCurThread->StartStackPointer;
	pThread->TermHandler			= spCurThread->TermHandler;
	pThread->LimitStackPointer		= spCurThread->LimitStackPointer;
	pThread->WaitObjectId			= spCurThread->WaitObjectId;
	pThread->WaitObjectType			= spCurThread->WaitObjectType;

	pThdStack = &(sStacks[0]);
	pThdStack += uiTID;
	pDst = &(pThdStack->Stack[THREAD_STACK_SIZE - 1]);

	pSrc = spCurThread->StartStackPointer;
	while (pSrc >= spCurStack) {
		*pDst-- = *pSrc--;
	}

	pThread->LastStackPointer	= pCurStack;
	pThread->Flags				|= THREAD_FLAG__RESERVED;
	pThread->Id					= uiTID;

	spCurThread->SaveToTID = uiTID;

	return TRUE;
}

BOOL ThdRestoreCurrentThread()
{
	PTHREAD			pTh;
	UINT32_PTR_T	pDst;
	UINT32_PTR_T	pSrc;
	PTHREAD_STACK	pThStack;

	pTh = &(sThreads[0]);
	pTh += spCurThread->SaveToTID;

	if ((pTh->Flags & THREAD_FLAG__RESERVED) == 0) {
		spCurThread->LastError = ERR__INVALID_SAVE_TARGET_THREAD;
		return FALSE;
	}

	spCurThread->Control				= pTh->Control;
	spCurThread->Counter				= pTh->Counter;
	spCurThread->Entry					= pTh->Entry;
	spCurThread->LastError				= pTh->LastError;
	spCurThread->Parameter				= pTh->Parameter;
	spCurThread->ReturnValue			= pTh->ReturnValue;
	spCurThread->SavedPC				= pTh->SavedPC;
	spCurThread->StartStackPointer		= pTh->StartStackPointer;
	
	if ((spCurThread->Flags & THREAD_FLAG__TERMINATING) == 0) {
		spCurThread->TermHandler		= pTh->TermHandler;
	}
	
	spCurThread->LimitStackPointer		= pTh->LimitStackPointer;
	spCurThread->WaitObjectId			= pTh->WaitObjectId;
	spCurThread->WaitObjectType			= pTh->WaitObjectType;

	spCurThread->LastStackPointer		= pTh->LastStackPointer;

	pThStack = &(sStacks[0]);
	pThStack += pTh->Id;
	pSrc = &(pThStack->Stack[THREAD_STACK_SIZE - 1]);

	pDst = spCurThread->StartStackPointer;
	while (pDst >= spCurThread->LastStackPointer)
	{
		*pDst-- = *pSrc--;
	}

	pTh->Flags							= THREAD_FLAG__NONE;
	pTh->State							= THREAD_STATE__EXIT;

	return TRUE;
}

BOOL ThdReleaseCurrentSaveTID()
{
	PTHREAD			pTh;

	pTh = &(sThreads[0]);
	pTh += spCurThread->SaveToTID;

	if ((pTh->Flags & THREAD_FLAG__RESERVED) == 0) {
		spCurThread->LastError = ERR__INVALID_SAVE_TARGET_THREAD;
		return FALSE;
	}

	pTh->State		= THREAD_STATE__EXIT;
	pTh->Flags		= THREAD_FLAG__NONE;

	return TRUE;
}
*/