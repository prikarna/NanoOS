/*
 * File    : Exception.h
 * Remark  : Exception and interrupt function definition
 *
 */

#ifndef EXCEPTION_H
#define EXCEPTION_H

#define INTERRUPT_OFFSET			0x10	// 16

void ExcReset();
void ExcNonMaskableInterrupt();
void ExcHardFault();
void ExcGeneralFault();
void ExcReserved();
void ExcSupervisorCall();
void ExcDebugMonitor();
void ExcPendSupervisorCall();
void ExcSystemTick();
void IntDefault();
void IntUSBLowPriorityOrCAN1_RX0();
void IntUSART3();
void IntTimer2();

#endif		// End of EXCEPTION_H
