/*
 * File    : Debug.c
 * Remark  : Mini debug console (viewer only) to view some 
 *           internal aspect of NanoOS.
 *
 */

#include "Sys\CortexM\Scb.h"
#include "Sys\CortexM\Systick.h"
#include "Sys\CortexM\Nvic.h"
#include "Sys\Stm\Usart.h"
#include "Sys\Stm\Rcc.h"
#include "Sys\Stm\Gpio.h"
#include "Sys\Stm\Timer.h"

#include "NanoOS.h"
#include "Thread.h"
#include "Service.h"

#define DBG_BUF_SIZE				64

extern unsigned int	_start_bss, _end_bss;

extern PTHREAD ThdGetCurrent();
extern PTHREAD ThdGetFirstThread();
extern PTHREAD_STACK ThdGetFirstStack();

UINT32_PTR_T		gpLastDbgStack;
UINT32_PTR_T		gpLastDbgFrameAddr;

static INT8_T		sBuf[DBG_BUF_SIZE];
static INT8_PTR_T	spBuf;
static UINT32_T		suBufCount;

static UINT32_T		suVar;
static UINT32_PTR_T	spStack;
static PTHREAD_STACK	spThdStack;
static PTHREAD		spThread;
static INT8_PTR_T	sszSubCmd1;

static UINT32_T		suDbgCode;
static UINT32_T		suDbgParm1;
static UINT32_T		suDbgParm2;
static UINT32_T		suDbgAddr;

void DbgLED(UINT32_T FlipCount, UINT32_T DelayCountAfter)
{
	UINT32_T	u;

	if ((RCC_GET_APB2_CTRL_CLK() & RCC_APB2_CTRL__IOPC) == 0) {
		RCC_ENA_APB2_CTRL_CLK(ENABLE, RCC_APB2_CTRL__IOPC);
		GPIO_SET_MODE_OUTPUT(GPIO_PORT__C, GPIO_PIN__13, GPIO_CNF_OUTPUT__GEN_PUSH_PULL, GPIO_MODE__OUTPUT_2MHZ);
	}

	for (u = 0; u < FlipCount; u++)
	{
		GPIO_WRITE(GPIO_PORT__C, GPIO_PIN__13, FALSE);
		suVar = 400000;
		while (suVar-- > 0);

		GPIO_WRITE(GPIO_PORT__C, GPIO_PIN__13, TRUE);
		suVar = 400000;
		while (suVar-- > 0);
	}

	suVar = DelayCountAfter;
	while (suVar-- > 0);
}

/*
void DbgDumpMem(const char* szSrc, const char * szMsg, UINT8_PTR_T pMemSrc, UINT32_T uMemLen)
{
	UINT32_T		u = 0;
	UINT8_PTR_T		p = pMemSrc;

	if (szSrc) {
		UaWriteString(szSrc);
		UaWriteString(": ");
	}
	UaWriteString(szMsg);
	UaWriteString(": ");

	for (u = 0; u < uMemLen; u++) {
		UaWriteChar('[');
		UaWriteHexa(TRUE, *p++, 2);
		UaWriteChar(']');
	}

	UaWriteString("\r\n");
}
*/

/*
void DbgDumpMem16(const char* szSrc, const char * szMsg, UINT8_PTR_T pMemSrc, UINT32_T uMemByteLen)
{
	UINT32_T		u = 0;
	UINT16_PTR_T	p = (UINT16_PTR_T) pMemSrc;
	UINT32_T		uDiv = 0, uRes = 0;

	if (szSrc) {
		UaWriteString(szSrc);
		UaWriteString(": ");
	}
	UaWriteString(szMsg);
	UaWriteString(": ");

	if ((!pMemSrc) || (uMemByteLen <= 0)) {
		UaWriteString("[NULL]\r\n");
		return;
	}

	uDiv = uMemByteLen / 2;
	for (u = 0; u < uDiv; u++) {
		UaWriteChar('[');
		UaWriteHexa(TRUE, *p++, 4);
		UaWriteChar(']');
	}

	uRes = uMemByteLen - (uDiv * 2);
	if (uRes) {
		UaWriteChar('[');
		UaWriteHexa(TRUE, *p++, 2);
		UaWriteChar(']');
	}

	UaWriteString("\r\n");
}

void DbgDumpMem32(const char* szSrc, const char * szMsg, UINT8_PTR_T pMemSrc, UINT32_T uMemByteLen)
{
	UINT32_T		u = 0;
	UINT32_PTR_T	p = (UINT32_PTR_T) pMemSrc;
	UINT32_T		uDiv = 0, uRes = 0;

	if (szSrc) {
		UaWriteString(szSrc);
		UaWriteString(": ");
	}
	UaWriteString(szMsg);
	UaWriteString(": ");

	if ((!pMemSrc) || (uMemByteLen <= 0)) {
		UaWriteString("[NULL]\r\n");
		return;
	}

	uDiv = uMemByteLen / 4;
	for (u = 0; u < uDiv; u++) {
		UaWriteChar('[');
		UaWriteHexa(TRUE, *p++, 8);
		UaWriteChar(']');
	}

	uRes = uMemByteLen - (uDiv * 4);
	if (uRes) {
		UaWriteChar('[');
		UaWriteHexa(TRUE, *p++, 8);
		UaWriteChar(']');
	}

	UaWriteString("\r\n");
}
*/

static void DbgDisplayThread()
{
	UaPrintf("\r\nThread slot %d :\r\n", suVar);
	UaPrintf(" Id              : %d\r\n", spThread->Id);
	UaPrintf(" StartStackPtr   : 0x%X\r\n", spThread->StartStackPtr);
	UaPrintf(" LastStackPtr    : 0x%X\r\n", spThread->LastStackPtr);
	UaPrintf(" LimitStackPtr   : 0x%X\r\n", spThread->LimitStackPtr);
	UaPrintf(" ProbeStackPtr   : 0x%X\r\n", spThread->ProbeStackPtr);
	UaPrintf(" State           : %d\r\n", spThread->State);
	UaPrintf(" Control         : %d\r\n", spThread->Control);
	UaPrintf(" TermHandler     : 0x%X\r\n", spThread->TermHandler);
	UaPrintf(" Counter         : %d\r\n", spThread->Counter);
	UaPrintf(" WaitObjectId    : %d\r\n", spThread->WaitObjectId);
	UaPrintf(" WaitObjectType  : %d\r\n", spThread->WaitObjectType);
	UaPrintf(" Flags           : 0x%X\r\n", spThread->Flags);
	UaPrintf(" Entry           : 0x%X\r\n", spThread->Entry);
	UaPrintf(" Parameter       : 0x%X\r\n", spThread->Parameter);
	UaPrintf(" ReturnValue     : %d\r\n", spThread->ReturnValue);
}

__attribute__((naked))
void DbgConsole(UINT32_T uTrapCode, UINT32_T uParam1, UINT32_T uParam2)
{
	__asm volatile 
		(
		"CPSID f;"
		"CPSID i;"
		);

	__asm volatile 
		(
		"MOV.W SP, %0;"
		"SUB SP, #64;"
		"SUB R7, SP, #32;"
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
		: "=r" (suDbgAddr), "=r" (suDbgCode), "=r" (suDbgParm1), "=r" (suDbgParm2)
		);

	UsbShutdown();

	NMemSet((UINT8_PTR_T) &sBuf, 0, DBG_BUF_SIZE);
	spBuf = &sBuf[0];
	suBufCount = 0;
	sszSubCmd1 = 0;

	UaWriteString("\r\nDBG>");

	while (TRUE) 
	{
		while (USART_IS_RX_NOT_EMPTY(USART3) == FALSE);
		*spBuf = USART_READ(USART3);
		suBufCount++;
		if (suBufCount >= DBG_BUF_SIZE) {
			NMemSet((UINT8_PTR_T) &sBuf, 0, DBG_BUF_SIZE);
			spBuf = &sBuf[0];
			suBufCount = 0;
			sszSubCmd1 = 0;
			UaWriteString("\r\nDBG>");
			continue;
		}

		if (*spBuf != '\r') {
			UaWriteChar(*spBuf);
			if (*spBuf == ' ') {
				*spBuf = '\0';
				if (sszSubCmd1 == 0) {
					sszSubCmd1 = spBuf;
					sszSubCmd1++;
				}
			}
			spBuf++;
			continue;
		}

		UaWriteString("\r\n");

		*spBuf = '\0';
		if (NStrCmp(&sBuf[0], "last_stack") == 0)
		{
			if ((gpLastDbgStack >= (UINT32_PTR_T) SRAM_ADDRESS) &&
				(gpLastDbgStack < (UINT32_PTR_T) (SRAM_ADDRESS + SRAM_SIZE - 1)))
			{
				spStack = gpLastDbgStack;
				for (suVar = 0; suVar < 16; suVar++)
				{
					UaPrintf(" [0x%X]: 0x%X\r\n", spStack, *spStack);
					spStack++;
				}
			}
			else
			{
				UaPrintf(" 0x%X\r\n", gpLastDbgStack);
			}
		}
		else if (NStrCmp(&sBuf[0], "last_frame") == 0)
		{
			UaPrintf(" 0x%X\r\n", gpLastDbgFrameAddr);
		}
		else if (NStrCmp(&sBuf[0], "thread") == 0)
		{
			spThread =  ThdGetFirstThread();
			suVar = ((UINT32_T) *sszSubCmd1) - 48;
			if ((suVar >= 0) && (suVar < MAX_NUMB_OF_THREADS))
			{
				spThread += suVar;
				DbgDisplayThread();
			}
			else
			{
				for (suVar = 0; suVar < MAX_NUMB_OF_THREADS; suVar++)
				{
					DbgDisplayThread();
					spThread++;
				}
			}
		}
		else if (NStrCmp(&sBuf[0], "stack") == 0)
		{
			spThread = ThdGetFirstThread();
			spThdStack = ThdGetFirstStack();

			suVar = ((UINT32_T) *sszSubCmd1) - 48;
			if ((suVar >= 0) && (suVar < MAX_NUMB_OF_THREADS))
			{
				spThread += suVar;
				spThdStack += suVar;

				spStack = spThread->LastStackPtr;
				if (spStack > spThread->StartStackPtr) 
				{
					UaPrintf(" Invalid stack pointer: start=0x%X, last=0x%X\r\n", 
						spThread->StartStackPtr, spThread->LastStackPtr);
				} 
				else 
				{
					UaPrintf("\r\nThread slot %d stack :\r\n", suVar);
					if (spStack == 0) 
					{
						spStack = &(spThdStack->Stack[THREAD_STACK_SIZE - 1]);
						for (suVar = 0; suVar < 16; suVar++)
						{
							UaPrintf(" *[0x%X]: 0x%X\r\n", spStack, *spStack);
							spStack--;
						}
					} 
					else 
					{
						while (spStack <= spThread->StartStackPtr)
						{
							UaPrintf(" [0x%X]: 0x%X\r\n", spStack, *spStack);
							spStack++;
						}
					}
				}
			}
			else
			{
				spStack = spThread->LastStackPtr;
				if (spStack > spThread->StartStackPtr) 
				{
					UaPrintf(" Invalid stack pointer: start=0x%X, last=0x%X\r\n", 
						spThread->StartStackPtr, spThread->LastStackPtr);
				} 
				else 
				{
					UaWriteString("\r\nThread slot 0 stack :\r\n");
					if (spStack == 0)
					{
						UaWriteString(" None. (last=0)\r\n");
					}
					else
					{
						while (spStack <= spThread->StartStackPtr)
						{
							UaPrintf(" [0x%X]: 0x%X\r\n", spStack, *spStack);
							spStack++;
						}
					}
				}
			}
		}
		else if (NStrCmp(&sBuf[0], "mem_cfg") == 0)
		{
			//UaPrintf("System SRAM address        : 0x%X\r\n", SYSTEM_SRAM_ADDRESS);

			suVar = ((UINT32_T) &_end_bss) - SYSTEM_SRAM_ADDRESS;
			UaPrintf("System SRAM usage          : %d bytes\r\n", suVar);

			//UaPrintf("System stack address       : 0x%X\r\n", SYSTEM_STACK_ADDRESS);
			
			suVar = SYSTEM_SRAM_SIZE - suVar;
			UaPrintf("System stack size          : %d bytes\r\n", suVar);
			
			//UaPrintf("Maximum number of threads  : %d\r\n", MAX_NUMB_OF_THREADS);
			//UaPrintf("Application SRAM address   : 0x%X\r\n", APP_SRAM_ADDRESS);
			//UaPrintf("Application SRAM size      : %d bytes\r\n", APP_SRAM_SIZE);
			//UaPrintf("Application stack size     : %d bytes\r\n", APP_STACK_SIZE);
			//UaPrintf("Application address        : 0x%X\r\n", APP_FLASH_ADDRESS);
		}
		//else if (NStrCmp(sBuf, "scb") == 0)
		//{
		//	UaPrintf(" ICSR  : 0x%X\r\n", IO_MEM32(SCB_ICSR));
		//	UaPrintf(" AIRCR : 0x%X\r\n", IO_MEM32(SCB_AIRCR));
		//	UaPrintf(" SHCSR : 0x%X\r\n", IO_MEM32(SCB_SHCSR));
		//}
		else if (NStrCmp(sBuf, "reset") == 0)
		{
			SCB_RESET_SYSTEM();
		}
		else if (NStrCmp(sBuf, "param") == 0)
		{
			UaPrintf(" Code    : 0x%X\r\n", suDbgCode);
			UaPrintf(" Address : 0x%X\r\n", suDbgAddr);
			UaPrintf(" Param 1 : 0x%X\r\n", suDbgParm1);
			UaPrintf(" Param 2 : 0x%X\r\n", suDbgParm2);
		}
		else if (NStrCmp(sBuf, "exc_prio") == 0)
		{
			UaPrintf(" SV Call      : %d\r\n", SCB_GET_SV_CALL_PRIO());
			UaPrintf(" Pend SV Call : %d\r\n", SCB_GET_PEND_SV_PRIO());
			UaPrintf(" Systick      : %d\r\n", SCB_GET_SYSTICK_PRIO());
			UaPrintf(" Mem. fault   : %d\r\n", SCB_GET_MEM_MGMT_FAULT_PRIO());
			UaPrintf(" Bus fault    : %d\r\n", SCB_GET_BUS_FAULT_PRIO());
			UaPrintf(" Usage fault  : %d\r\n", SCB_GET_USAGE_FAULT_PRIO());

			for (suVar = 0; suVar < NVIC_IRQ__MAXIMUM; suVar++)
			{
				UaPrintf(" IRQ %d       : %d\r\n", suVar, NVIC_GET_IRQ_PRIO(suVar));
			}
		}
		else
		{
			UaPrintf("Unknown command %s", sBuf);
		}

		NMemSet((UINT8_PTR_T) &sBuf, 0, DBG_BUF_SIZE);
		spBuf = &sBuf[0];
		suBufCount = 0;
		sszSubCmd1 = 0;
		UaWriteString("\r\nDBG>");
	}
}