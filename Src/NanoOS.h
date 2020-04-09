/*
 * File    : NanoOS.h
 * Remark  : The definition of functions exported by NanoOS modules. 
 *           Most of these functions are not supposed to be used by 
 *           application directly, application should use NanoOS API 
 *           instead.
 *           If an application, for some reason, need to call these 
 *           functions, it should call them from 'exception or interrupt' 
 *           level execution (int. priority <= 6), e.g., interrupt handler.
 *
 */

#ifndef NANOOS_H
#define NANOOS_H

#define PROG_NAME_STR		"NanoOS"
#define PROG_VER			0x0102
#define PROG_MJ_VER			((PROG_VER >> 8) & 0xFF)
#define PROG_MN_VER			(PROG_VER & 0xFF)
#define TGT_CHIP_STR		"Stm32f10XXX"

#include "Sys\Type.h"

/*
 * Usart
 */
void UaInitialize();
void UaWriteChar(UINT8_T ch);
UINT32_T UaWriteString(const char * szString);
UINT32_T UaWriteHexa(UINT8_T uIsHexCapital, UINT32_T uHex, UINT32_T uDigit);
UINT32_T UaPrintf(const char * szFormat, ...);


/*
 * Debug
 */
void DbgDumpMem(const char* szSrc, const char * szMsg, UINT8_PTR_T pMemSrc, UINT32_T uMemLen);
void DbgLED(UINT32_T FlipCount, UINT32_T DelayCountAfter);
void DbgConsole(UINT32_T uTrapCode, UINT32_T uParam1, UINT32_T uParam2);

#define DbgBreak()				__asm volatile ("bkpt #1")

#ifdef _DEBUG
# define DBG_PRINTF(szFmt, ...)							UaPrintf(szFmt, __VA_ARGS__)
# define DBG_OUT(szString)								UaWriteString(szString)
# define DBG_DUMPMEM(szSrc, szMsg, pMem, uMemByteLen)	DbgDumpMem(szSrc, szMsg, pMem, uMemByteLen)
# define DBG_BREAK()									__asm volatile ("bkpt #1")
# define DBG_DUMP_STACK(szSrc, uVar, pStack, uLength)							\
	for (uVar = 0; uVar < uLength; uVar++) {									\
		UaPrintf("%s: [%d] [0x%X] = 0x%X\r\n", szSrc, uVar, pStack, *pStack);	\
		pStack++;																\
	}
# define DBG_LED(FlipCount, DelayAfter)					DbgLED(FlipCount, DelayAfter)
#else
# define DBG_PRINTF(szFmt, ...)
# define DBG_OUT(szString)
# define DBG_DUMPMEM(szSrc, szMsg, pMem, uMemByteLen)
# define DBG_BREAK()									__asm volatile ("NOP")
# define DBG_DUMP_STACK(szSrc, uVar, pStack, uLength)
# define DBG_LED(Flip, Delay)
#endif


/*
 * System clock
 */
UINT8_T ClkSetSystemClockToMaxSpeed();
UINT32_T ClkGetSystemClock();
UINT32_T ClkGetAPB1Divider();
UINT32_T ClkGetAHBDivider();


/*
 * Memory
 */
void NMemCopy(UINT8_PTR_T pDst, UINT8_PTR_T pSrc, UINT32_T uLen);
void NMemSet(UINT8_PTR_T pMem, UINT8_PTR_T uSet, UINT32_T uLen);
void NMemCopyToPaddedBuffer(UINT8_PTR_T pDestination, UINT8_PTR_T pSource, UINT32_T uSourceLength);
void NMemCopyFromPaddedBuffer(UINT8_PTR_T pDestination, UINT8_PTR_T pSource, UINT32_T uDestinationByteLen);

int NStrLen(const char * pStr);
int NStrCmp(const char * pStr1, const char * pStr2);
int NStrCopy(char * pBuffer, const char * pStr, int iBufferLen);


/*
 * Console
 */
void ConInitialize();
UINT8_T ConStart();
void ConTerminate();
void ConHandleUsartInt(UINT8_T ch);

/*
 * Thread
 */
#include "Error.h"
#include "Thread.h"

void ThdInitialize();

UINT8_T ThdCreate(
				  UINT32_PTR_T		pStartStack,
				  UINT8_T			uiThreadControl,
				  UINT8_T			fIsSuspended,
				  THREAD_ENTRY_TYPE ThreadEntry, 
				  void *			pParameter, 
				  UINT32_PTR_T		pThreadId
				  );

UINT8_T ThdGetReturnValue(UINT32_T uThreadId, int * piThreadReturnValue);
UINT8_T ThdGetState(UINT32_T uThreadId, UINT8_PTR_T pThreadState, PTHREAD pRequestingThread);
UINT8_T	ThdTerminate(UINT32_T uThreadId);
UINT8_T ThdGetCurrentId(UINT32_PTR_T pRetTID);
UINT8_T ThdGetControlType(UINT32_T uThreadId, UINT8_PTR_T pControlType);
void ThdExit();

BOOL ThdWait(
			 UINT8_T		uiWaitType, 
			 UINT8_T		uiObjectType, 
			 UINT32_T		uiObjectId, 
			 UINT32_T		uiMilliSec
			 );

BOOL ThdGetCurrentControlType(UINT8_PTR_T puiControlType);
UINT8_T ThdGetCurrentState(UINT8_PTR_T puiState);
BOOL ThdSuspend(UINT32_T uiThreadId);
BOOL ThdResume(UINT32_T uiThreadId);
BOOL ThdSetTerminationHandler(TERMINATION_HANDLER_TYPE TermHandler);
TERMINATION_HANDLER_TYPE ThdGetCurrentTerminationHandler();
void ThdSetLastError(UINT32_T uiError);
UINT32_T ThdGetLastError();
BOOL ThdSaveCurrentThread(UINT32_PTR_T pCurStack);
BOOL ThdRestoreCurrentThread();
BOOL ThdReleaseCurrentSaveTID();


/*
 * Usb
 */
void UsbInitialize();
UINT8_T UsbSend(UINT8_PTR_T pBuffer, UINT32_T uiBufferByteLength, BOOL fWait);
BOOL UsbRequestReceive();
UINT8_T UsbReceive(UINT8_PTR_T pBuffer, UINT32_T uiBuffLength, UINT32_PTR_T puiReceivedDataLength);
void UsbCancelReceive();
BOOL UsbIsReady();
void UsbShutdown();


/* 
 * Core
 */
#define TRAP__NMI									0x00000010
#define TRAP__CUR_STACK_PTR_GT_START_STACK_PTR		0x00000011
#define TRAP__UNEXPECTED_CUR_THREAD_STATE			0x00000012
#define TRAP__INVALID_CUR_THREAD					0x00000013
#define TRAP__INVALID_CUR_THREAD_ID					0x00000014
#define TRAP__STACK_SCAN_LENGTH_EXCEEDED			0x00000015
#define TRAP__INVALID_SERVICE_INDEX					0x00000016
#define TRAP__RESERVED_EXCEPTION					0x00000017
#define TRAP__DEBUG_EXCEPTION						0x00000018
#define TRAP__UNHANDLED_INTERRUPT					0x00000019
#define TRAP__HARD_FAULT							0x0000001A
#define TRAP__GENERAL_FAULT							0x0000001B
#define TRAP__UNHANDLED_GENERAL_FAULT				0x0000001C
#define TRAP__UNEXPECTED_EXECUTION					0x0000001D
#define TRAP__POSSIBLE_DATA_CORRUPTION				0x0000001E
#define TRAP__INVALID_CURRENT_STACK					0x0000001F
#define TRAP__INVALID_SERVICE_STACK					0x00000020

#define TRAP__UNKNOWN_USB_DESCRIPTOR_TYPE			0x00000101
#define TRAP__UNEXPECTED_USB_ENDPOINT_REGISTER		0x00000102
#define TRAP__UNKNOWN_USB_INTERRUPT					0x00000103

void CrTrapExecution(
				   UINT32_T Code, 
				   UINT32_T Param1, 
				   UINT32_T Param2
				   );
UINT32_T CrGetLastFault();
void CrSetFaultStatus(UINT32_T uStat);
BOOL CrIsAppRunning();

#ifdef _DEBUG
# define TRAP_EXEC(Expression, Code, Param1, Param2)	\
	if ((Expression)) DbgConsole(Code, Param1, Param2)
#else
# define TRAP_EXEC(Expresion, Code, Param1, Param2)		\
	if ((Expresion)) CrTrapExecution(Code, Param1, Param2)
#endif


/*
 * Event
 */
void EvtInitialize();
BOOL EvtCreate(UINT32_PTR_T puiEventId, PTHREAD pRequestingThread);
BOOL EvtClose(UINT32_T uiEventId, PTHREAD pRequestingThread);
BOOL EvtSet(UINT32_T uiEventId, PTHREAD pRequestingThread);
BOOL EvtReset(UINT32_T uiEventId, PTHREAD pRequestingThread);
BOOL EvtGetState(UINT32_T uiEventId, UINT16_PTR_T puiState, PTHREAD pRequestingThread);



/*
 * Config
 */
#include "Config.h"
BOOL CfgLoad();
BOOL CfgSave();
PCONFIG CfgGet();


/*
 * Flash
 */
BOOL FlsUnlock();
BOOL FlsLock();
BOOL FlsGetAddressFromPageNo(UINT32_T uiPageNo, UINT32_PTR_T *ppRetPageAddress);
BOOL FlsGetPageNoFromAddress(UINT32_PTR_T pAddress, UINT32_PTR_T pRetPageNo);
BOOL FlsErase(UINT32_T uiStartPageNo, UINT32_T uiNumbOfPages);
BOOL FlsProgram(UINT16_PTR_T pDestination, UINT16_PTR_T pSource, UINT32_T uiLength);
BOOL FlsProgramFromPaddedSource(UINT16_PTR_T pDestination, UINT32_PTR_T pSource, UINT32_T uiNumberOfOperation);


/*
 * Interrupt
 */
#include "Interrupt.h"
BOOL ExcSetInterrupt(INTERRUPT_HANDLER_TYPE IntHandler);


/*
 * Utils
 */
typedef void (* PRINT_CHAR_TYPE)(UINT8_T);
UINT32_T PrnWriteString(PRINT_CHAR_TYPE PrintChar, const char *szString);
UINT32_T PrnWriteHexa(PRINT_CHAR_TYPE PrintChar, UINT8_T uIsHexCapital, UINT32_T uHex, UINT32_T uDigit);
UINT32_T PrnVPrintf(PRINT_CHAR_TYPE PrintChar, const char *szFormat, va_list argList);
UINT32_T PrnPrintf(PRINT_CHAR_TYPE PrintChar, const char *szFormat, ...);

#endif  // End of NANOOS_H
