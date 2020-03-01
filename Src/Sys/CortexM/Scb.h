/*
 * File    : Scb.h
 * Remark  : Macro to access Cortex-M SCB (System Control Block). 
 *
 */

#ifndef SCB_H
#define SCB_H

#include "..\Type.h"
#include "..\Bit.h"

#define SCB_BASE				0xE000ED00
#define SCB_ACTLR				0xE000E008
#define SCB_CPUID				(SCB_BASE + 0x00)
#define SCB_ICSR				(SCB_BASE + 0x04)
#define SCB_VTOR				(SCB_BASE + 0x08)
#define SCB_AIRCR				(SCB_BASE + 0x0C)
#define SCB_SCR					(SCB_BASE + 0x10)
#define SCB_CCR					(SCB_BASE + 0x14)
#define SCB_SHPR1				(SCB_BASE + 0x18)
#define SCB_SHPR2				(SCB_BASE + 0x1C)
#define SCB_SHPR3				(SCB_BASE + 0x20)
#define SCB_SHCSR				(SCB_BASE + 0x24)
#define SCB_CFSR				(SCB_BASE + 0x28)
#define SCB_HFSR				(SCB_BASE + 0x2C)
#define SCB_MMAR				(SCB_BASE + 0x34)
#define SCB_BFAR				(SCB_BASE + 0x38)

#define SCB_DISABLE_IT_INSTRUCTION_FOLDING(IsTrue)		ENABLE_IO_BIT(SCB_ACTLR, BIT_2, IsTrue)

#define SCB_DISABLE_WRITE_BUFFER_USE(IsTrue)			ENABLE_IO_BIT(SCB_ACTLR, BIT_1, IsTrue)

#define SCB_DISABLE_INTERRUPTIONS_LATENCY(IsTrue)		ENABLE_IO_BIT(SCB_ACTLR, BIT_0, IsTrue)

#define SCB_GET_CPU_IMPLEMENTER_ID()			GET_IO_BITS(SCB_CPUID, BIT_24, BITMASK_8)
#define SCB_GET_CPU_VARIANT_NUMB()				GET_IO_BITS(SCB_CPUID, BIT_20, BITMASK_4)
#define SCB_GET_CPU_PART_NO()					GET_IO_BITS(SCB_CPUID, BIT_4, BITMASK_12)
#define SCB_GET_CPU_REVISION_NO()				GET_IO_BITS(SCB_CPUID, BIT_0, BITMASK_4)
#define SCB_GET_CPUID()							IO_MEM32(SCB_CPUID)

#define SCB_GET_INT_CTRL_AND_STATE()			IO_MEM32(SCB_ICSR)

#define SCB_SET_PEND_NMI()						SET_IO_BIT(SCB_ICSR, BIT_31)

#define SCB_SET_PEND_SV()						SET_IO_BIT(SCB_ICSR, BIT_28)
#define SCB_IS_PEND_SV()						GET_IO_BIT(SCB_ICSR, BIT_28)
#define SCB_CLR_PEND_SV()						SET_IO_BIT(SCB_ICSR, BIT_27)

#define SCB_SET_PEND_SYSTICK()					SET_IO_BIT(SCB_ICSR, BIT_26)
#define SCB_IS_PEND_SYSTICK()					GET_IO_BIT(SCB_ICSR, BIT_26)
#define SCB_CLR_PEND_SYSTICK()					SET_IO_BIT(SCB_ICSR, BIT_25)

#define SCB_IS_ISR_PENDING()					GET_IO_BIT(SCB_ICSR, BIT_22)
#define SCB_GET_PENDING_VECTOR()				GET_IO_BITS(SCB_ICSR, BIT_12, BITMASK_10)
#define SCB_IS_RET_TO_BASE_LEVEL()				GET_IO_BIT(SCB_ICSR, BIT_11)

#define SCB_VECTOR__MANAGEMENT_FAULT			4
#define SCB_VECTOR__BUS_FAULT					5
#define SCB_VECTOR__USAGE_FAULT					6
#define SCB_VECTOR__SV_CALL						11
#define SCB_VECTOR__PEND_SV_CALL				14
#define SCB_VECTOR__SYSTICK						15
#define SCB_GET_ACTIVE_VECTOR()					GET_IO_BITS(SCB_ICSR, BIT_0, BITMASK_9)
#define SCB_SET_ACTIVE_VECTOR(Val)				SET_IO_BITS(SCB_ICSR, BIT_0, BITMASK_9, Val)

#define SCB_GET_VECTOR_TABLE_OFFSET()			IO_MEM32(SCB_VTOR)
#define SCB_SET_VECTOR_TABLE_OFFSET(Val)		IO_MEM32(SCB_VTOR) = Val

#define SCB_ENA_APP_INT_AND_RST_CTL()			SET_IO_BITS(SCB_AIRCR, BIT_16, BITMASK_16, 0x5FA)
#define SCB_SET_INT_GRP_FIELD(ScbGrpField)		SET_IO_BITS(SCB_AIRCR, BIT_8, BITMASK_3, ScbGrpField)
#define SCB_GET_INT_GRP_FIELD()					SET_IO_BITS(SCB_AIRCR, BIT_8, BITMASK_3)

#define SCB_RESET_SYSTEM()						IO_MEM32(SCB_AIRCR) = ((((UINT32_T) 0x5FA) << BIT_16) | (BITHEX_2))

#define SCB_ENA_EVT_ON_PENDING_BIT(Enable)		ENABLE_IO_BIT(SCB_SCR, BIT_4, Enable)
#define SCB_ENA_DEEP_SLEEP(Enable)				ENABLE_IO_BIT(SCB_SCR, BIT_2, Enable)
#define SCB_ENA_SLEEP_ON_EXIT(Enable)			ENABLE_IO_BIT(SCB_SCR, BIT_1, Enable)
#define SCB_IS_SLEEP_ON_EXIT_ENABLED()			GET_IO_BIT(SCB_SCR, BIT_1)

#define SCB_ENA_8_BYTE_STACK_ALIGNMENT(Enable)					ENABLE_IO_BIT(SCB_CCR, BIT_9, Enable)
#define SCB_ENA_IGNORE_DATA_BUS_FAULT(Enable)					ENABLE_IO_BIT(SCB_CCR, BIT_8, Enable)
#define SCB_ENA_DIVIDE_BY_ZERO_TRAP(Enable)						ENABLE_IO_BIT(SCB_CCR, BIT_4, Enable)
#define SCB_ENA_UNALIGN_TRAP(Enable)							ENABLE_IO_BIT(SCB_CCR, BIT_3, Enable)
#define SCB_ENA_USER_TO_SET_SOFT_PEND(Enable)					ENABLE_IO_BIT(SCB_CCR, BIT_1, Enable)
#define SCB_ENA_PROC_ENTER_THREAD_MODE_FROM_ANY_LEVEL(Enable)	ENABLE_IO_BIT(SCB_CCR, BIT_0, Enable)

#define SCB_SET_USAGE_FAULT_PRIO(ScbPrio)			SET_IO_BITS(SCB_SHPR1, BIT_20, BITMASK_4, ScbPrio)
#define SCB_GET_USAGE_FAULT_PRIO()					GET_IO_BITS(SCB_SHPR1, BIT_20, BITMASK_4)
#define SCB_SET_BUS_FAULT_PRIO(ScbPrio)				SET_IO_BITS(SCB_SHPR1, BIT_12, BITMASK_4, ScbPrio)
#define SCB_GET_BUS_FAULT_PRIO()					GET_IO_BITS(SCB_SHPR1, BIT_12, BITMASK_4)
#define SCB_SET_MEM_MGMT_FAULT_PRIO(ScbPrio)		SET_IO_BITS(SCB_SHPR1, BIT_4, BITMASK_4, ScbPrio)
#define SCB_GET_MEM_MGMT_FAULT_PRIO()				GET_IO_BITS(SCB_SHPR1, BIT_4, BITMASK_4)

#define SCB_SET_SV_CALL_PRIO(ScbPrio)				SET_IO_BITS(SCB_SHPR2, BIT_28, BITMASK_4, ScbPrio)
#define SCB_GET_SV_CALL_PRIO()						GET_IO_BITS(SCB_SHPR2, BIT_28, BITMASK_4)

#define SCB_SET_SYSTICK_PRIO(ScbPrio)				SET_IO_BITS(SCB_SHPR3, BIT_28, BITMASK_4, ScbPrio)
#define SCB_GET_SYSTICK_PRIO()						GET_IO_BITS(SCB_SHPR3, BIT_28, BITMASK_4)

#define SCB_SET_PEND_SV_PRIO(ScbPrio)				SET_IO_BITS(SCB_SHPR3, BIT_20, BITMASK_4, ScbPrio)
#define SCB_GET_PEND_SV_PRIO()						GET_IO_BITS(SCB_SHPR3, BIT_20, BITMASK_4)

#define SCB_ENA_USAGE_FAULT_EXCP(Enable)			ENABLE_IO_BIT(SCB_SHCSR, BIT_18, Enable)
#define SCB_ENA_BUS_FAULT_EXCP(Enable)				ENABLE_IO_BIT(SCB_SHCSR, BIT_17, Enable)
#define SCB_ENA_MEM_FAULT_EXC(Enable)				ENABLE_IO_BIT(SCB_SHCSR, BIT_16, Enable)

#define SCB_IS_SV_CALL_PENDED()						GET_IO_BIT(SCB_SHCSR, BIT_15)
#define SCB_CLR_SV_CALL_PENDED()					CLR_IO_BIT(SCB_SHCSR, BIT_15)
#define SCB_SET_SV_CALL_PENDED()					SET_IO_BIT(SCB_SHCSR, BIT_15)

#define SCB_IS_BUS_FAULT_PENDED()					GET_IO_BIT(SCB_SHCSR, BIT_14)
#define SCB_CLR_BUS_FAULT_PENDED()					SET_IO_BIT(SCB_SHCSR, BIT_14)

#define SCB_IS_MEM_FAULT_PENDED()					GET_IO_BIT(SCB_SHCSR, BIT_13)
#define SCB_CLR_MEM_FAULT_PENDED()					CLR_IO_BIT(SCB_SHCSR, BIT_13)

#define SCB_IS_USAGE_FAULT_PENDED()					GET_IO_BIT(SCB_SHCSR, BIT_12)
#define SCB_CLR_USAGE_FAULT_PENDED()				CLR_IO_BIT(SCB_SHCSR, BIT_12)

#define SCB_IS_SYSTICK_ACTIVE()						GET_IO_BIT(SCB_SHCSR, BIT_11)
#define SCB_SET_SYSTICK_ACTIVE()					SET_IO_BIT(SCB_SHCSR, BIT_11)
#define SCB_CLR_SYSTICK_ACTIVE()					CLR_IO_BIT(SCB_SHCSR, BIT_11)

#define SCB_IS_PEND_SV_ACTIVE()						GET_IO_BIT(SCB_SHCSR, BIT_10)
#define SCB_CLR_PEND_SV_ACTIVE()					CLR_IO_BIT(SCB_SHCSR, BIT_10)

#define SCB_IS_DBG_MON_ACTIVE()						GET_IO_BIT(SCB_SHCSR, BIT_8)
#define SCB_CLR_DBG_MON_ACTIVE()					CLR_IO_BIT(SCB_SHCSR, BIT_8)
#define SCB_SET_DBG_MON_ACTIVE()					SET_IO_BIT(SCB_SHCSR, BIT_8)

#define SCB_IS_SV_CALL_ACTIVE()						GET_IO_BIT(SCB_SHCSR, BIT_7)
#define SCB_SET_SV_CALL_ACTIVE()					SET_IO_BIT(SCB_SHCSR, BIT_7)
#define SCB_CLR_SV_CALL_ACTIVE()					CLR_IO_BIT(SCB_SHCSR, BIT_7)

#define SCB_IS_USAGE_FAULT_ACTIVE()					GET_IO_BIT(SCB_SHCSR, BIT_3)
#define SCB_CLR_USAGE_FAULT_ACTIVE()				CLR_IO_BIT(SCB_SHCSR, BIT_3)

#define SCB_IS_BUS_FAULT_ACTIVE()					GET_IO_BIT(SCB_SHCSR, BIT_1)
#define SCB_CLR_BUS_FAULT_ACTIVE()					CLR_IO_BIT(SCB_SHCSR, BIT_1)

#define SCB_IS_MEM_FAULT_ACTIVE()					GET_IO_BIT(SCB_SHCSR, BIT_0)
#define SCB_CLR_MEM_FAULT_ACTIVE()					CLR_IO_BIT(SCB_SHCSR, BIT_0)

#define SCB_HARDFAULT__DEBUG_VT						BITHEX_31
#define SCB_HARDFAULT__FORCED						BITHEX_30
#define SCB_HARDFAULT__VECTOR_TABLE					BITHEX_1
#define SCB_HARDFAULT__ALL							(BITHEX_31 | BITHEX_30 | BITHEX_1)
#define SCB_GET_HARDFAULT_STATUS()					IO_MEM32(SCB_HFSR)

#define SCB_CLR_HARDFAULT_STATUS(ScbHardFaultBits)	IO_MEM32(SCB_HFSR) |= ScbHardFaultBits

#define SCB_USAGE_FAULT__DEVIDE_BY_ZERO				BITHEX_25
#define SCB_USAGE_FAULT__UNALIGNED_USAGE			BITHEX_24
#define SCB_USAGE_FAULT__NO_CO_PROCESSOR			BITHEX_19
#define SCB_USAGE_FAULT__INVALID_PC_LOAD			BITHEX_18
#define SCB_USAGE_FAULT__INVALID_STATE				BITHEX_17
#define SCB_USAGE_FAULT__UNDEF_INSTRUCTION			BITHEX_16
#define SCB_USAGE_FAULT__ALL						\
	(BITHEX_25 | BITHEX_24 | BITHEX_19 | BITHEX_18 | BITHEX_17 | BITHEX_16)

#define SCB_BUS_FAULT__VALID_FAULT_ADDRESS			BITHEX_15
#define SCB_BUS_FAULT__EXC_ENTRY_STACKING_ERROR		BITHEX_12
#define SCB_BUS_FAULT__EXC_RETURN_STACKING_ERROR	BITHEX_11
#define SCB_BUS_FAULT__IMPRECISE_ERROR_DATA			BITHEX_10
#define SCB_BUS_FAULT__PRECISE_ERROR_DATA			BITHEX_9
#define SCB_BUS_FAULT__INSTRUCTION_ERROR			BITHEX_8
#define SCB_BUS_FAULT__ALL							\
	(BITHEX_15 | BITHEX_12 | BITHEX_11 | BITHEX_10 | BITHEX_9 | BITHEX_8)

#define SCB_MEM_FAULT__VALID_FAULT_ADDRESS			BITHEX_7
#define SCB_MEM_FAULT__EXC_ENTRY_STACKING_ERROR		BITHEX_4
#define SCB_MEM_FAULT__EXC_RETURN_STACKING_ERROR	BITHEX_3
#define SCB_MEM_FAULT__DATA_ACCESS_VIOLATION		BITHEX_1
#define SCB_MEM_FAULT__INSTRUCTION_ACCESS_VIOLATION	BITHEX_0
#define SCB_MEM_FAULT__ALL							\
	(BITHEX_7 | BITHEX_4 | BITHEX_3 | BITHEX_1 | BITHEX_0)

#define SCB_GET_FAULT_STATUS()						IO_MEM32(SCB_CFSR)

#define SCB_CLR_USAGE_FAULT_STATUS(ScbUsageFault)	IO_MEM32(SCB_CFSR) |= (ScbUsageFault)
#define SCB_CLR_FAULT_STATUS(ScbGenFault)			IO_MEM32(SCB_CFSR) &= (~(ScbGenFault))

#define SCB_CLR_BUS_FAULT_ADDRESS()					IO_MEM32(SCB_BFAR) &= (~(SCB_BUS_FAULT__ALL))
#define SCB_GET_BUS_FAULT_ADDRESS()					IO_MEM32(SCB_BFAR)

#define SCB_CLR_MEM_FAULT_ADDRESS()					IO_MEM32(SCB_MMAR) &= (~(SCB_MEM_FAULT__ALL))
#define SCB_GET_MEM_FAULT_ADDRESS()					IO_MEM32(SCB_MMAR)

#define SCB_CLR_GENERAL_FAULT_STATUS()								\
	SCB_CLR_USAGE_FAULT_STATUS(SCB_USAGE_FAULT__ALL);				\
	SCB_CLR_FAULT_STATUS((SCB_BUS_FAULT__ALL | SCB_MEM_FAULT__ALL))

#define SCB_GENERAL_FAULTS							(SCB_USAGE_FAULT__ALL | SCB_BUS_FAULT__ALL | SCB_MEM_FAULT__ALL)

#define SCB_GET_GENERAL_FAULT_STATUS()				IO_MEM32(SCB_CFSR)

#endif  // End of SCB_H