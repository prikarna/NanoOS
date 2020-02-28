/*
 * File    : Console.c
 * Remark  : Built-in simple console module for NanoOS, to control 
 *           or view some aspect of NanoOS like thread. This console 
 *           has no security facility (no logon or logout, no password 
 *           and etc.).
 *           Console use serial TTL as its input and output.
 *
 */

#include "Sys\CortexM\Scb.h"
#include "Sys\CortexM\Systick.h"
#include "Sys\CortexM\Nvic.h"
#include "Sys\Stm\Gpio.h"
#include "Sys\Stm\Usb.h"
#include "Sys\Stm\Rcc.h"
#include "Sys\Stm\Flash.h"

#include "NanoOS.h"
#include "Console.h"
#include "Thread.h"
#include "Config.h"
#include "Address.h"
#include "ServiceDef.h"
#include "UsbData.h"

extern UINT32_T		guAppTID;

static INT8_T		sInBuf[CONS__INPUT_BUFFER_LENGTH];
static INT8_PTR_T	spInBuf;
static UINT32_T		suBufCount;

static UINT32_T		suTID;
static INT8_PTR_T	sszCmd;
static INT8_PTR_T	sszSubCmd1;
static INT8_PTR_T	sszSubCmd2;
static PCONFIG		spCfg;

static BOOL			sfConAct;

static UINT32_T		suStressLock;

static const char*	sSzCmds[] =
{
	"ver",			"help",			"exit",		"led",		"run",		"thread",		"term",		"suspend",
	"resume",		"stress",		"dump",		"reset",	"config",	"auto_run",		"app_priv",	"debug",	
	"lock_stress",	"usb_send",		"usb_recv",	0
};

static UINT8_T		suRunCmd;

void ConInitialize()
{
	NMemSet((UINT8_PTR_T) &sInBuf[0], 0, CONS__INPUT_BUFFER_LENGTH);
	spInBuf = &sInBuf[0];
	suBufCount = 0;
	spCfg = CfgGet();
	suTID = 0;
	sfConAct = FALSE;
	suRunCmd = 0;
}

static void ConDisplayHelp()
{
	UaWriteString("Available commands: \r\n");
	UaWriteString(" ver                      to display version of NanoOS and console.\r\n");
	UaWriteString(" help                     to display this help.\r\n");
	UaWriteString(" exit                     to exit from console.\r\n");
	UaWriteString(" led                      to run test LED.\r\n");
	UaWriteString(" run [flash|sram]         to run application in flash or sram.\r\n");
	UaWriteString(" thread [0|1|2|...]       to display thread(s) status.\r\n");
	UaWriteString(" term [0|1|2|...]         to terminate thread.\r\n");
	UaWriteString(" suspend [0|1|2|...]      to suspend thread.\r\n");
	UaWriteString(" resume [0|1|2|...]       to resume thread.\r\n");
	UaWriteString(" stress                   to run simple stress test.\r\n");
	UaWriteString(" lock_stress              to run simple stress test with lock.\r\n");
	UaWriteString(" dump                     to display application binary data in flash.\r\n");
	UaWriteString(" reset                    to reset this system.\r\n");
	UaWriteString(" config [save|show|load]  to show, load and save NanoOS configuration.\r\n");
	UaWriteString(" auto_run [ena|dis]       to enable or disable application auto run.\r\n");
	UaWriteString(" app_priv [priv|unpriv]   to set application control: privileged or unprivileged.\r\n");
	UaWriteString(" debug                    to enter debug console (no switch back to current console).\r\n");
}

static void ConDumpAppBinData()
{
	UINT16_PTR_T	pMem = (UINT16_PTR_T) APP_FLASH_ADDRESS;
	UINT32_T		u, y;
	UINT16_T		uDat;

	UaPrintf("Application memory address: 0x%X\r\n", pMem);
	UaWriteString("Display 256 bytes application binary data in half-word (16 bit):\r\n");
	UaWriteString("===============================================================\r\n");
	for (u = 0; u < 16; u++) {
		for (y = 0; y < 16; y++) {
			uDat = *pMem;
			UaPrintf("[%4X]", uDat);
			pMem++;
		}
		UaWriteString("\r\n");
	}
	UaWriteString("---------------------------------------------------------------\r\n");
}

static BOOL ConGetThreadStatus(UINT32_T uiTID, UINT8_PTR_T puiStatus)
{
	BOOL fRes = 0;

	if (!puiStatus) return FALSE;

	CALL_SVC_WITH_TWO_PARAMS(SVC__GET_THREAD_STATUS, fRes, uiTID, puiStatus);

	return fRes;
}

static BOOL ConTerminateThread(UINT32_T uiTID)
{
	BOOL fRes = 0;

	CALL_SVC_WITH_ONE_PARAM(SVC__TERMINATE_THREAD, fRes, uiTID);

	return fRes;
}

static BOOL ConGetCurrentThreadId(UINT32_PTR_T puiTID)
{
	BOOL fRes = FALSE;

	if (!puiTID) return FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__GET_CURRENT_THREAD_ID, fRes, puiTID);

	return fRes;
}

static BOOL ConSuspendThread(UINT32_T uiThreadId)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__SUSPEND_THREAD, fRes, uiThreadId);

	return fRes;
}

static BOOL ConResumeThread(UINT32_T uiThreadId)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__RESUME_THREAD, fRes, uiThreadId);

	return fRes;
}

static void ConDisplayThreadStatus(UINT32_T uiThreadId)
{
	BOOL	fRes = FALSE;
	UINT8_T	s;

	fRes = ConGetThreadStatus(uiThreadId, &s);
	if (fRes) {
		UaPrintf("Thread %d status\t: ", uiThreadId);

		switch (s)
		{
		case THREAD_STATUS__EXIT:
			UaWriteString("Exit.\r\n");
			break;

		case THREAD_STATUS__TERMINATED:
			UaWriteString("Terminated.\r\n");
			break;

		case THREAD_STATUS__SLEEP:
			UaWriteString("Sleep.\r\n");
			break;

		case THREAD_STATUS__SUSPENDED:
			UaWriteString("Suspended.\r\n");
			break;

		case THREAD_STATUS__ACTIVE:
			UaWriteString("Active.\r\n");
			break;

		default:
			UaPrintf("Unknown (%d).\r\n", s);
			break;
		}
	} else {
		UaPrintf("Error get thread %d state!\r\n", uiThreadId);
	}
}

static void ConDisplayAllThreadStatus()
{
	UINT32_T	u;

	for (u = 0; u < MAX_NUMB_OF_THREADS; u++) {
		ConDisplayThreadStatus(u);
	}
}

static void ConInitializeSpinLock(UINT32_PTR_T pLock)
{
	if (!pLock) return;
	*pLock = 0;
}

static BOOL ConAcquireSpinLock(UINT32_PTR_T pLock)
{
	BOOL	fRes = FALSE;
	if (!pLock) return FALSE;

	do {
		CALL_SVC_WITH_ONE_PARAM(SVC__ACQUIRE_SPIN_LOCK, fRes, pLock);
	} while (fRes == FALSE);

	return fRes;
}

static BOOL ConReleaseSpinLock(UINT32_PTR_T pLock)
{
	BOOL	fRes = FALSE;

	if (!pLock) return FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__RELEASE_SPIN_LOCK, fRes, pLock);

	return fRes;
}

/*
static BOOL ConSwitchToNextThread()
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ZERO_PARAM(SVC__SWITCH_TO_NEXT_THREAD, fRes);

	return fRes;
}

static BOOL ConWriteToUsbSer(UINT8_PTR_T pData, UINT32_T uiDatLen)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_TWO_PARAMS(SVC__WRITE_USB_SER, fRes, pData, uiDatLen);

	return fRes;
}

static BOOL ConReadFromUsbSer(UINT8_PTR_T pDat, UINT32_T uiDatLen, UINT32_PTR_T pRecvDatLen)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_THREE_PARAMS(SVC__READ_USB_SER, fRes, pDat, uiDatLen, pRecvDatLen);

	return fRes;
}
*/

static BOOL ConSetTermHandler(TERMINATION_HANDLER_TYPE TermHandler)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__SET_TERMINATION_HANDLER, fRes, TermHandler);

	return fRes;
}

static void ConConsoleTermHandler(UINT32_T uReason)
{
	sfConAct = FALSE;
	UaPrintf("Console terminated. (0x%X)\r\n", uReason);
}

void ConHandleUsartInt(UINT8_T ch)
{
	UINT8_T		fRes = FALSE;

	if (sfConAct == FALSE) {
		fRes = ConStart();
		if (!fRes) {
			UaWriteString("Fail to start console!\r\n");
		}
		return;
	}

	if (suRunCmd)
		return;

	if (suBufCount >= (CONS__INPUT_BUFFER_LENGTH - 1)) {
		spInBuf = &sInBuf[0];
		suBufCount = 0;
		NMemSet((UINT8_PTR_T) sInBuf, 0, CONS__INPUT_BUFFER_LENGTH);
	}

	if (ch == '\r') 
	{
		if (suRunCmd == 0)
			suRunCmd = 1;
	} 
	else 
	{
		*spInBuf++ = ch;
		suBufCount++;
		UaWriteChar(ch);
	}
}

int ConTestLED(void * pParam)
{
	UINT32_T	u;
	UINT32_T	uCount;
	UINT32_T	uMax = 20;
	UINT32_T	uTID = 0;

	ConGetCurrentThreadId(&uTID);
	if (pParam) {
		uMax = *((UINT32_PTR_T) pParam);
	}

	UaPrintf("[TID=%d]%s: Starting... (Max.=%d)\r\n", uTID, __FUNCTION__, uMax);
	
	RCC_ENA_APB2_CTRL_CLK(ENABLE, RCC_APB2_CTRL__IOPC);

	GPIO_SET_MODE_OUTPUT(GPIO_PORT__C, GPIO_PIN__13, GPIO_CNF_OUTPUT__GEN_PUSH_PULL, GPIO_MODE__OUTPUT_2MHZ);

	for (u = 0; u < uMax; u++) {
		GPIO_WRITE(GPIO_PORT__C, GPIO_PIN__13, LOW);
		uCount = 500000;
		while (uCount-- > 0);

		GPIO_WRITE(GPIO_PORT__C, GPIO_PIN__13, HIGH);
		uCount = 500000;
		while (uCount-- > 0);

		UaPrintf("[TID=%d]%s: Count = %d\r\n", uTID, __FUNCTION__, u);

		if (u == 20) ThdExit();
	}

	UaPrintf("[TID=%d]%s: End\r\n", uTID, __FUNCTION__);

	return (int) u;
}

static int ConTestStress(void * pParam)
{
	UINT32_T	uMax = 10000;
	UINT32_T	uiTID;

	ConGetCurrentThreadId(&uiTID);

	for (UINT32_T u = 0; u < uMax; u++) {
		UaPrintf("[TID=%d]%s: Hello world. (%d)\r\n", uiTID, __FUNCTION__, u);
	}

	return uMax;
}

static int ConTestStressWithLock(void * pParam)
{
	UINT32_T	uMax = 1000;
	UINT32_T	uiTID;

	ConGetCurrentThreadId(&uiTID);

	for (UINT32_T u = 0; u < uMax; u++) {
		ConAcquireSpinLock(&suStressLock);
		UaPrintf("[TID=%d]%s: Hello world. (%d)\r\n", uiTID, __FUNCTION__, u);
		ConReleaseSpinLock(&suStressLock);
	}

	return uMax;
}

static void ConDisplayVersion()
{
	UINT32_T	uClk;

	uClk = ClkGetSystemClock();
	UaPrintf("%s Version %d.%d on %s chipset.\r\n", PROG_NAME_STR, PROG_MJ_VER, PROG_MN_VER, TGT_CHIP_STR);
	UaPrintf("System running at %d Hz (%d MHz).\r\n", uClk, (uClk / 1000000));
	UaPrintf("Console Version %d.%d\r\n", CONS_MJ_VER, CONS_MN_VER);
}

static void ConDisplayConfig()
{
	if (!spCfg) return;

	if (spCfg->AutoRun) {
		UaWriteString("Auto run         : Yes\r\n");
	} else {
		UaWriteString("Auto run         : No\r\n");
	}
	if (spCfg->Control == THREAD_CONTROL__PRIVILEGED) {
		UaWriteString("Control          : Privileged\r\n");
	} else {
		UaWriteString("Control          : UnPrivileged\r\n");
	}
	
	UaPrintf("Application name : %s\r\n", spCfg->AppName);
}

static void ConRunAppInSRAM()
{
	UINT32_T		uiAppSRAMSize = 0;
	BOOL			fRes = FALSE;
	
	uiAppSRAMSize = (APP_SRAM_SIZE - (THREAD_STACK_SIZE * STACK_ALIGN));

	NMemCopy((UINT8_PTR_T) APP_SRAM_ADDRESS, (UINT8_PTR_T) APP_FLASH_ADDRESS, uiAppSRAMSize);

	fRes = ThdCreate(
					0,
					spCfg->Control,
					FALSE,
					(THREAD_ENTRY_TYPE) (APP_SRAM_ADDRESS + 1),
					0,
					&guAppTID
					);
	if (!fRes) {
		UaWriteString("Fail to start application in SRAM!\r\n");
	}
}

static int ConGetCommand()
{
	UINT8_T	u = 0;
	int	i = 0;
	INT8_PTR_T	p = (INT8_PTR_T) &sInBuf[0];

	sszCmd = p;
	sszSubCmd1 = 0;
	sszSubCmd2 = 0;
	while (*p != '\0') {
		if (*p != ' ') {
			p++;
		} else {
			*p++ = '\0';
			if (sszSubCmd1 == 0) {
				sszSubCmd1 = p;
			} else if (sszSubCmd2 == 0) {
				sszSubCmd2 = p;
			} else {
			}
		}
	}

	if (sszCmd == 0) return -1;

	while (sSzCmds[u] != 0)
	{
		i = NStrCmp((const INT8_PTR_T) sSzCmds[u], sszCmd);
		if (i == 0) break;
		u++;
	}

	if (sSzCmds[u] == 0)
		return -1;

	return (int) u;
}

int Console(void * pParameter)
{
	UINT8_T			uRes = 0;
	int				iCmd = 0;
	UINT32_T		uClk;
	UINT32_T		uThArg = 40;
	UINT32_T		uTID;

	sfConAct = TRUE;

	ConSetTermHandler(ConConsoleTermHandler);

	uClk = ClkGetSystemClock();
	UaPrintf("%s Version %d.%d on %s chipset.\r\n", PROG_NAME_STR, PROG_MJ_VER, PROG_MN_VER, TGT_CHIP_STR);
	UaPrintf("System running at %d Hz (%d MHz).\r\n", uClk, (uClk / 1000000));
	UaWriteString("#>");

	while (sfConAct)
	{
		while (suRunCmd == 0);
		if (sInBuf[0] == 0) {
			suRunCmd = 0;
			UaWriteString("\r\n#>");
			continue;
		}

		UaWriteString("\r\n");

		iCmd = ConGetCommand();
		switch (iCmd)
		{
		case CONS_CMD__VER:
			ConDisplayVersion();
			break;

		case CONS_CMD__HELP:
			ConDisplayHelp();
			break;

		case CONS_CMD__EXIT:
			sfConAct = FALSE;
			break;

		case CONS_CMD__LED:
			uRes = ThdCreate(
							0, 
							THREAD_CONTROL__PRIVILEGED, 
							FALSE, 
							ConTestLED, 
							(void *) &uThArg, 
							0
							);
			if (uRes == 0)
				UaWriteString("Fail to run test LED!\r\n");
			break;

		case CONS_CMD__RUN:
			if (sszSubCmd1 == 0) {
				uRes = ThdCreate(
								0, 
								spCfg->Control,
								FALSE, 
								(THREAD_ENTRY_TYPE) APP_FLASH_RUN_ADDRESS, 
								0, 
								&guAppTID
								);
				if (uRes == 0) {
					UaWriteString("Fail to run user program in flash!\r\n");
				}
			} else {
				if (NStrCmp(sszSubCmd1, "flash") == 0)
				{
					uRes = ThdCreate(
									0,
									spCfg->Control, 
									FALSE, 
									(THREAD_ENTRY_TYPE) APP_FLASH_RUN_ADDRESS, 
									0, 
									&guAppTID
									);
					if (uRes == 0) {
						UaWriteString("Fail to run user program in flash!\r\n");
					}
				}
				else if (NStrCmp(sszSubCmd1, "sram") == 0)
				{
					ConRunAppInSRAM();
				}
				else
				{
					UaPrintf("Invalid run option %s\r\n", sszSubCmd1);
				}
			}
			break;

		case CONS_CMD__THREAD:
			if (sszSubCmd1 == 0) {
				ConDisplayAllThreadStatus();
			} else {
				uTID = ((UINT32_T) *sszSubCmd1) - ((UINT32_T) '0');
				if (uTID < MAX_NUMB_OF_THREADS)
				{
					ConDisplayThreadStatus(uTID);
				}
				else
				{
					UaPrintf("Invalid thread id %c\r\n", *sszSubCmd1);
				}
			}
			break;

		case CONS_CMD__TERM:
			if (sszSubCmd1 == 0) {
				UaPrintf("Must provide thread id.\r\n");
			} else {
				uTID = ((UINT32_T) (*sszSubCmd1)) - ((UINT32_T) '0');
				if (uTID < MAX_NUMB_OF_THREADS)
				{
					if (uTID == suTID) {
						UaPrintf("Can't terminate this thread!\r\n");
					} else {
						uRes = ConTerminateThread(uTID);
					}
				}
				else
				{
					UaPrintf("Invalid thread id %c\r\n", *sszSubCmd1);
				}
			}
			break;

		case CONS_CMD__SUSPEND:
			if (sszSubCmd1 == 0) {
				UaPrintf("Must provide thread id.\r\n");
			} else {
				uTID = ((UINT32_T) *sszSubCmd1) - ((UINT32_T) '0');
				if (uTID < MAX_NUMB_OF_THREADS)
				{
					if (uTID == suTID) {
						UaPrintf("Can't suspend this thread!\r\n");
					} else {
						uRes = ConSuspendThread(uTID);
					}
				}
				else
				{
					UaPrintf("Invalid thread id %c\r\n", *sszSubCmd1);
				}
			}
			break;

		case CONS_CMD__RESUME:
			if (sszSubCmd1 == 0) {
				UaPrintf("Must provide thread id.\r\n");
			} else {
				uTID = ((UINT32_T) *sszSubCmd1) - ((UINT32_T) '0');
				if (uTID < MAX_NUMB_OF_THREADS)
				{
					if (uTID == suTID) {
						UaPrintf("Can't resume this thread!\r\n");
					} else {
						uRes = ConResumeThread(uTID);
					}
				}
				else
				{
					UaPrintf("Invalid thread id %c\r\n", *sszSubCmd1);
				}
			}
			break;

		case CONS_CMD__STRESS:
			ThdCreate(
					0,
					THREAD_CONTROL__PRIVILEGED, 
					FALSE,
					ConTestStress, 
					0, 
					0
					);
			ThdCreate(
					0, 
					THREAD_CONTROL__UNPRIVILEGED,
					FALSE,
					ConTestStress, 
					0, 
					0
					);
			ThdCreate(
					0, 
					THREAD_CONTROL__UNPRIVILEGED,
					FALSE,
					ConTestStress, 
					0, 
					0
					);
			break;

		case CONS_CMD__DUMP:
			ConDumpAppBinData();
			break;

		case CONS_CMD__RESET:
			UsbShutdown();
			UaWriteString("System reset...\r\n");
			SCB_RESET_SYSTEM();
			break;

		case CONS_CMD__CONFIG:
			if (sszSubCmd1 == 0) {
				ConDisplayConfig();
			} else {
				if (NStrCmp(sszSubCmd1, "show") == 0)
				{
					ConDisplayConfig();
				}
				else if (NStrCmp(sszSubCmd1, "save") == 0)
				{
					UaWriteString("Saving config... ");
					uRes = CfgSave();
					if (!uRes) {
						UaWriteString(" fail!\r\n");
					} else {
						UaWriteString(" success.\r\n");
					}
				}
				else if (NStrCmp(sszSubCmd1, "load") == 0)
				{
					uRes = CfgLoad();
					UaPrintf("Config loaded.\r\n");
				}
				else
				{
					UaPrintf("Invalid option %s\r\n", sszSubCmd1);
				}
			}
			break;

		case CONS_CMD__AUTO_RUN:
			if (sszSubCmd1 == 0) {
				UaPrintf("Type option ena/dis to enable/disable auto run.\r\n");
			} else {
				if (NStrCmp(sszSubCmd1, "ena") == 0)
				{
					spCfg->AutoRun = TRUE;
					UaPrintf("Auto run enable.\r\n");
				}
				else if (NStrCmp(sszSubCmd1, "dis") == 0)
				{
					spCfg->AutoRun = FALSE;
					UaPrintf("Auto run disabled.\r\n");
				}
				else
				{
					UaPrintf("Invalid option %s\r\n", sszSubCmd1);
				}
			}
			break;

		case CONS_CMD__APP_PRIV:
			if (sszSubCmd1 == 0) {
				UaPrintf("Type option priv/unpriv to set application privilege.\r\n");
			} else {
				if (NStrCmp(sszSubCmd1, "priv") == 0)
				{
					spCfg->Control = THREAD_CONTROL__PRIVILEGED;
					UaPrintf("Application set to privileged level.\r\n");
				}
				else if (NStrCmp(sszSubCmd1, "unpriv") == 0)
				{
					spCfg->AutoRun = FALSE;
					UaPrintf("Application set to privileged level.\r\n");
				}
				else
				{
					UaPrintf("Invalid option %s\r\n", sszSubCmd1);
				}
			}
			break;

		case CONS_CMD__DEBUG:
			DbgConsole(0x10D, 0, 0);
			break;

		case CONS_CMD__LOCK_STRESS:
			ConInitializeSpinLock(&suStressLock);
			ThdCreate(
					0,
					THREAD_CONTROL__PRIVILEGED, 
					FALSE,
					ConTestStressWithLock, 
					0, 
					0
					);
			ThdCreate(
					0, 
					THREAD_CONTROL__UNPRIVILEGED,
					FALSE,
					ConTestStressWithLock, 
					0, 
					0
					);
			ThdCreate(
					0, 
					THREAD_CONTROL__UNPRIVILEGED,
					FALSE,
					ConTestStressWithLock, 
					0, 
					0
					);
			break;

			/*
		case CONS_CMD__USB_SEND:
			pWr = "This comming from console.";
			uRes = ConWriteToUsbSer((UINT8_PTR_T) pWr, NStrLen(pWr));
			if (uRes) {
				UaWriteString("OK.\r\n");
			} else {
				UaWriteString("failed!\r\n");
			}
			break;

		case CONS_CMD__USB_RECV:
			while (TRUE) {
				NMemSet((UINT8_PTR_T) &sUsbDat[0], 0, sizeof(sUsbDat));
				uRes = ConReadFromUsbSer(&sUsbDat[0], sizeof(sUsbDat), &uClk);
				if (uRes) {
					if (sUsbDat[0] == '\r') break;
					UaPrintf("OK: data[0] = %c (%d), len=%d\r\n", sUsbDat[0], sUsbDat[0], uClk);
				} else {
					UaWriteString("failed!\r\n");
					break;
				}
			}
			break;
			*/

		default:
			UaPrintf("%s is invalid command. Type help to show available commands.\r\n", sInBuf);
			break;
		}

		NMemSet((UINT8_PTR_T) sInBuf, 0, CONS__INPUT_BUFFER_LENGTH);
		spInBuf = &sInBuf[0];
		suBufCount = 0;

		if (sfConAct) {
			UaWriteString("#>");
		}

		suRunCmd = 0;
	}

	UaWriteString("Bye.\r\n");

	return 1;
}

UINT8_T ConStart()
{
	UINT8_T			uRes = 0;
	
	uRes = ThdCreate(0, THREAD_CONTROL__PRIVILEGED, FALSE, Console, 0, &suTID);

	return uRes;
}

/*
void ConTerminate()
{
	sfConAct = FALSE;
	ThdTerminate(suTID);
}
*/

__attribute__ ((section(".nanoos_application")))
int App(void * pParm)
{
	__asm volatile ("NOP");
	__asm volatile ("NOP");

	return 0;
}