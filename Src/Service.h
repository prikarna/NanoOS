/*
 * File    : Service.h
 * Remark  : Service parameter and function definition
 *
 */

#ifndef SERVICE_H
#define SERVICE_H

#include "Thread.h"

#define SVC_MAX_PARAMS			8

struct _SVC_PARAM {
	UINT32_T		Index;
	UINT32_T		ReturnValue;
	UINT32_T		Reserved2;
	UINT32_T		Reserved3;
	UINT32_T		R12;
	UINT32_T		LR;
	UINT32_T		PC;
	UINT32_T		PSR;
	UINT32_T		Params[SVC_MAX_PARAMS];
}__attribute__((packed));

typedef struct _SVC_PARAM		SVC_PARAM, *PSVC_PARAM;

typedef void (* SERVICE_CALL_TYPE)(void);

void SvcZero();

void SvcGetOSVersion();
void SvcGetOSName();
void SvcGetSystemClock();

void SvcReserved();

void SvcDebugCharOut();

void SvcIsUsbSerReady();
void SvcWriteUsbSer();
void SvcReadUsbSer();
void SvcCancelReadUsbSer();

void SvcCreateThread();
void SvcTerminateThread();
void SvcExitThread();
void SvcGetThreadReturnValue();
void SvcGetThreadStatus();
void SvcGetCurrentThreadId();
void SvcSleep();
void SvcSuspendThread();
void SvcResumeThread();
void SvcWaitForObject();
void SvcSwitchToNextThread();

void SvcCreateEvent();
void SvcSetEvent();
void SvcResetEvent();
void SvcCloseEvent();

void SvcSetTerminationHandler();
void SvcGetFaultAddress();
void SvcGetLastFault();

void SvcSetLastError();
void SvcGetLastError();

void SvcSetIntHandler();
//void SvcSetUsbIntHandler();

void SvcLoadConfig();
void SvcSaveConfig();

void SvcLockFlash();
void SvcUnlockFlash();
void SvcEraseFlash();
void SvcProgramFlash();

void SvcAcquireSpinLock();
void SvcReleaseSpinLock();

void SvcSystemReset();

void SvcSetProbe();
void SvcResetProbe();

#endif  // End of SERVICE_H
