/*
 * File    : ServiceDef.h
 * Remark  : Service number and call service definition used by NanoOS API
 *
 */

#ifndef SERVICE_DEF_H
#define SERVICE_DEF_H

#define SVC__ZERO							0
#define SVC__GET_OS_VERSION					1
#define SVC__GET_OS_NAME					2
#define SVC__GET_SYSTEM_CLOCK				3
#define SVC__RESERVED4						4
#define SVC__DEBUG_CHAR_OUT					5
#define SVC__RESERVED6						6
#define SVC__IS_USB_SER_READY				7
#define SVC__WRITE_USB_SER					8
#define SVC__READ_USB_SER					9
#define SVC__CANCEL_READ_USB_SER			10
#define SVC__RESERVED11						11
#define SVC__CREATE_THREAD					12
#define SVC__TERMINATE_THREAD				13
#define SVC__EXIT_THREAD					14
#define SVC__GET_THREAD_RETURN_VALUE		15
#define SVC__GET_THREAD_STATUS				16
#define SVC__GET_CURRENT_THREAD_ID			17
#define SVC__SLEEP							18
#define SVC__SUSPEND_THREAD					19
#define SVC__RESUME_THREAD					20
#define SVC__WAIT_FOR_OBJECT				21
#define SVC__SWITCH_TO_NEXT_THREAD			22
#define SVC__RESERVED23						23
#define SVC__RESERVED24						24
#define SVC__RESERVED25						25
#define SVC__RESERVED26						26
#define SVC__CREATE_EVENT					27
#define SVC__SET_EVENT						28
#define SVC__RESET_EVENT					29
#define SVC__CLOSE_EVENT					30
#define SVC__RESERVED31						31
#define SVC__RESERVED32						32
#define SVC__SET_TERMINATION_HANDLER		33
#define SVC__GET_FAULT_ADDRESS				34
#define SVC__SET_LAST_ERROR					35
#define SVC__GET_LAST_ERROR					36
#define SVC__GET_LAST_FAULT					37
#define SVC__SET_INT_HANDLER				38
#define SVC__RESERVED39						39
#define SVC__RESERVED40						40
#define SVC__LOAD_CONFIG					41
#define SVC__SAVE_CONFIG					42
#define SVC__RESERVED43						43
#define SVC__RESERVED44						44
#define SVC__UNLOCK_FLASH					45
#define SVC__LOCK_FLASH						46
#define SVC__ERASE_FLASH					47
#define SVC__PROGRAM_FLASH					48
#define SVC__RESERVED49						49
#define SVC__RESERVED50						50
#define SVC__ACQUIRE_SPIN_LOCK				51
#define SVC__RELEASE_SPIN_LOCK				52
#define SVC__RESERVED53						53
#define SVC__RESET_SYSTEM					54
#define SVC__RESERVED55						55
#define SVC__SET_PROBE						56
#define SVC__RESET_PROBE					57
#define SVC__RESERVED58						58
#define SVC__MAXIMUM						59

#define ASM_VOLATILE			__asm volatile

#define CALL_SVC(ServiceIndex, ReturnValueVar)	\
	ASM_VOLATILE (								\
			"MOV.W R0, %0;"						\
			"SVC #0;"							\
			"MOV.W %0, R1;"						\
			: "=r" (ReturnValueVar)				\
			: "r" (ServiceIndex)				\
			)

#define PUSH_SVC_PARAM(Param)	\
	ASM_VOLATILE (				\
			"MOV.W R0, %0;"		\
			"PUSH {R0};"		\
			:					\
			:					\
				"r" (Param)		\
			)

#define POP_SVC_PARAM()		ASM_VOLATILE ("POP {R0};")

#define SAVE_SVC_REGS()		ASM_VOLATILE ("PUSH {LR};")
#define RESTORE_SVC_REGS()	ASM_VOLATILE ("POP {LR};")

#define CALL_SVC_WITH_ZERO_PARAM(				\
								ServiceIndex,	\
								ReturnValueVar	\
								)				\
	SAVE_SVC_REGS();							\
	CALL_SVC(ServiceIndex, ReturnValueVar);		\
	RESTORE_SVC_REGS()

#define CALL_SVC_WITH_ONE_PARAM(				\
								ServiceIndex,	\
								ReturnValueVar,	\
								Param			\
								)				\
	SAVE_SVC_REGS();							\
	PUSH_SVC_PARAM(Param);						\
	CALL_SVC(ServiceIndex, ReturnValueVar);		\
	POP_SVC_PARAM();							\
	RESTORE_SVC_REGS()

#define CALL_SVC_WITH_TWO_PARAMS(				\
								ServiceIndex,	\
								ReturnValueVar,	\
								Param0,			\
								Param1			\
								)				\
	SAVE_SVC_REGS();							\
	PUSH_SVC_PARAM(Param1);						\
	PUSH_SVC_PARAM(Param0);						\
	CALL_SVC(ServiceIndex, ReturnValueVar);		\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	RESTORE_SVC_REGS()

#define CALL_SVC_WITH_THREE_PARAMS(				\
								ServiceIndex,	\
								ReturnValueVar,	\
								Param0,			\
								Param1,			\
								Param2			\
								)				\
	SAVE_SVC_REGS();							\
	PUSH_SVC_PARAM(Param2);						\
	PUSH_SVC_PARAM(Param1);						\
	PUSH_SVC_PARAM(Param0);						\
	CALL_SVC(ServiceIndex, ReturnValueVar);		\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	RESTORE_SVC_REGS()

#define CALL_SVC_WITH_FOUR_PARAMS(				\
								ServiceIndex,	\
								ReturnValueVar,	\
								Param0,			\
								Param1,			\
								Param2,			\
								Param3			\
								)				\
	SAVE_SVC_REGS();							\
	PUSH_SVC_PARAM(Param3);						\
	PUSH_SVC_PARAM(Param2);						\
	PUSH_SVC_PARAM(Param1);						\
	PUSH_SVC_PARAM(Param0);						\
	CALL_SVC(ServiceIndex, ReturnValueVar);		\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	RESTORE_SVC_REGS()

#define CALL_SVC_WITH_FIVE_PARAMS(				\
								ServiceIndex,	\
								ReturnValueVar,	\
								Param0,			\
								Param1,			\
								Param2,			\
								Param3,			\
								Param4			\
								)				\
	SAVE_SVC_REGS();							\
	PUSH_SVC_PARAM(Param4);						\
	PUSH_SVC_PARAM(Param3);						\
	PUSH_SVC_PARAM(Param2);						\
	PUSH_SVC_PARAM(Param1);						\
	PUSH_SVC_PARAM(Param0);						\
	CALL_SVC(ServiceIndex, ReturnValueVar);		\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	RESTORE_SVC_REGS()

#define CALL_SVC_WITH_SIX_PARAMS(				\
								ServiceIndex,	\
								ReturnValueVar,	\
								Param0,			\
								Param1,			\
								Param2,			\
								Param3,			\
								Param4,			\
								Param5			\
								)				\
	SAVE_SVC_REGS();							\
	PUSH_SVC_PARAM(Param5);						\
	PUSH_SVC_PARAM(Param4);						\
	PUSH_SVC_PARAM(Param3);						\
	PUSH_SVC_PARAM(Param2);						\
	PUSH_SVC_PARAM(Param1);						\
	PUSH_SVC_PARAM(Param0);						\
	CALL_SVC(ServiceIndex, ReturnValueVar);		\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	POP_SVC_PARAM();							\
	RESTORE_SVC_REGS()

#endif  // End of SERVICE_DEF_H
