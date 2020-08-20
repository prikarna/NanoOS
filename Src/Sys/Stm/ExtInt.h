/*
 * File    : ExtInt.h
 * Remark  : Macro to access External Interrupt controller on chip.
 *           Fow now this is STM32F10XXX chipset.
 *
 */

#ifndef EXTI_H
#define EXTI_H

#include "..\Type.h"
#include "..\Bit.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "ChipAddr.h"

#define EXTI_IMR				(EXTI_ADDRESS + 0x00)
#define EXTI_EMR				(EXTI_ADDRESS + 0x04)
#define EXTI_RSTR				(EXTI_ADDRESS + 0x08)
#define EXTI_FSTR				(EXTI_ADDRESS + 0x0C)
#define EXTI_SWIER				(EXTI_ADDRESS + 0x10)
#define EXTI_PR					(EXTI_ADDRESS + 0x14)

#define EXTI_LINE_0				BITHEX_0
#define EXTI_LINE_1				BITHEX_1
#define EXTI_LINE_2				BITHEX_2
#define EXTI_LINE_3				BITHEX_3
#define EXTI_LINE_4				BITHEX_4
#define EXTI_LINE_5				BITHEX_5
#define EXTI_LINE_6				BITHEX_6
#define EXTI_LINE_7				BITHEX_7
#define EXTI_LINE_8				BITHEX_8
#define EXTI_LINE_9				BITHEX_9
#define EXTI_LINE_10			BITHEX_10
#define EXTI_LINE_11			BITHEX_11
#define EXTI_LINE_12			BITHEX_12
#define EXTI_LINE_13			BITHEX_13
#define EXTI_LINE_14			BITHEX_14
#define EXTI_LINE_15			BITHEX_15
#define EXTI_LINE_16			BITHEX_16
#define EXTI_LINE_17			BITHEX_17
#define EXTI_LINE_18			BITHEX_18
#define EXTI_LINE_19			BITHEX_19

#define EXTI_ENA_INTR(Ena, ExtILines)					\
	(Ena) ? (IO_MEM32((EXTI_IMR)) |= ExtILines) : (IO_MEM32((EXTI_IMR)) &= ~(ExtILines))

#define EXTI_ENA_EVT(Ena, ExtILines)					\
	(Ena) ? (IO_MEM32((EXTI_EMR)) |= ExtILines) : (IO_MEM32((EXTI_EMR)) &= ~(ExtILines))

#define EXTI_ENA_RAISING_EDGE_TRIG(Ena, ExtILines)		\
	(Ena) ? (IO_MEM32((EXTI_RSTR)) |= ExtILines) : (IO_MEM32((EXTI_RSTR)) &= ~(ExtILines))

#define EXTI_ENA_FALLING_EDGE_TRIG(Ena, ExtILines)		\
	(Ena) ? (IO_MEM32((EXTI_FSTR)) |= ExtILines) : (IO_MEM32((EXTI_FSTR)) &= ~(ExtILines))

#define EXTI_TRIG_INT(ExtILines)						IO_MEM32((EXTI_SWIER)) |= ExtILines

#define EXTI_CLR_PEND_INT(ExtILines)					IO_MEM32((EXTI_PR)) |= ExtILines

#define EXTI_GET_PEND_INT()								(IO_MEM32((EXTI_PR)))

#ifdef __cplusplus
}
#endif

#endif  // End of EXTI_H
