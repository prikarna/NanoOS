/*
 * File    : Thread.h
 * Remark  : Contain thread definition, used by thread.c
 *
 */

#ifndef THREAD_H
#define THREAD_H

#include "Sys\Type.h"
#include "ThreadDef.h"
#include "Exception.h"
#include "Address.h"

#define STACK_ALIGN						sizeof(UINT32_T)

#define THREAD_STATE__EXIT				1
#define THREAD_STATE__TERMINATED		2
#define THREAD_STATE__QUEUEING			3
#define THREAD_STATE__RUNNING			4
#define THREAD_STATE__WAITING			5
#define THREAD_STATE__SLEEP				6
#define THREAD_STATE__WAIT_FOR_OBJECT	7
#define THREAD_STATE__SUSPENDED			8

#define MAX_NUMB_OF_THREADS				4

//#define THREAD_STACK_SIZE				256
#define THREAD_STACK_SIZE				512

struct _THREAD_STACK {
	UINT32_T	Stack[THREAD_STACK_SIZE];
}__attribute__((packed));

typedef struct _THREAD_STACK	THREAD_STACK, *PTHREAD_STACK;

#define TERMINATING_COUNT			2000		// Approx. 2 sec.

#define THREAD_FLAG__NONE			0x00000000
#define THREAD_FLAG__TERMINATING	0x00000001
#define THREAD_FLAG__RESERVED		0x00000002
#define THREAD_FLAG__PROBING		0x00000004
#define THREAD_FLAG__IO_RECEIVE		0x00000010
#define THREAD_FLAG__IO_SEND		0x00000020

#define THREAD_WAIT_TYPE__SLEEP			0
#define THREAD_WAIT_TYPE__OBJECT		1
#define THREAD_WAIT_TYPE__IO_INTERRUPT	2

struct _THREAD {
	struct _THREAD *			FLink;		// Forward linkage
	struct _THREAD *			BLink;		// Backward linkage
	UINT32_T					Id;
	THREAD_ENTRY_TYPE			Entry;
	int							ReturnValue;
	void *						Parameter;
	UINT8_T						Control;
	UINT8_T						State;
	UINT32_T					Counter;
	UINT8_T						WaitObjectType;
	UINT32_T					WaitObjectId;
	UINT32_T					SleepPC;
	UINT32_T					Flags;
	TERMINATION_HANDLER_TYPE	TermHandler;
	UINT32_T					LastError;
	UINT32_T					ProbePC;
	UINT32_T					ProbeLR;
	UINT32_T					ProbeFrameAddress;
	UINT32_PTR_T				ProbeStackPtr;
	UINT32_PTR_T				StartStackPtr;
	UINT32_PTR_T				LastStackPtr;
	UINT32_PTR_T				LimitStackPtr;
}__attribute__((packed));

typedef struct _THREAD	THREAD, *PTHREAD;

#define EXC_RETURN						0xFFFFFFF9

struct _EXC_STACK_FRAME {
	UINT32_T		R0;
	UINT32_T		R1;
	UINT32_T		R2;
	UINT32_T		R3;
	UINT32_T		R12;
	UINT32_T		LR;
	UINT32_T		PC;
	UINT32_T		PSR;
}__attribute__((packed));

typedef struct _EXC_STACK_FRAME	EXC_STACK_FRAME, *PEXC_STACK_FRAME;

struct _INITIAL_THREAD_STACK {
	UINT32_T			FrameAddress;
	UINT32_T			ExcReturn;
	EXC_STACK_FRAME		ExcFrame;
}__attribute__((packed));

typedef struct _INITIAL_THREAD_STACK	INITIAL_THREAD_STACK, *PINITIAL_THREAD_STACK;

#define SIZEOF_INITIAL_STACK()			(sizeof(INITIAL_THREAD_STACK) / STACK_ALIGN)

#define GET_STACK_POINTER(StackPointerVar)			\
	__asm volatile ("MOV.W %0, SP" : "=r" (StackPointerVar))

#define GET_FRAME_ADDRESS(FrameAddressVar)			\
	__asm volatile ("MOV.W %0, R7" : "=r" (FrameAddressVar))

#define THREAD_TICK_COUNT__LEVEL_3		900
#define THREAD_TICK_COUNT__LEVEL_2		300
#define THREAD_TICK_COUNT__LEVEL_1		100
#define THREAD_TICK_COUNT__LEVEL_0		90

#define BEGIN_EXCEPTION()				__asm volatile ("PUSH {R7, LR}")
#define END_EXCEPTION()					__asm volatile ("POP {R7, PC}")
#define GET_IPSR(Var)					__asm volatile ("MRS %0, IPSR" : "=r" (Var))
#define GET_CONTROL(Var)				__asm volatile ("MRS %0, CONTROL" : "=r" (Var))
#define SET_CONTROL(Var)				__asm volatile ("MSR CONTROL, %0" : : "r" (Var))
#define CLEAR_EXCLUSIVE()				__asm volatile ("CLREX")

#define SET_TO_SYSTEM_STACK()			\
	__asm volatile						\
		(								\
		"MOV.W SP, %0;"					\
		"SUB SP, #128;"					\
		"MOV.W R7, SP;"					\
		: : "r" (SYSTEM_STACK_ADDRESS)	\
		)

//#define THREAD_STACK_LIMIT_DELTA		96
#define THREAD_STACK_LIMIT_DELTA		64

#define TIMER_PRESCALE					360
#define TIMER_AUTO_RELOAD				200

#endif  // End of THREAD_H
