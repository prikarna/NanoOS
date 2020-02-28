/*
 * File    : SysTick.h
 * Remark  : Macro to access Cortex-M System tick controller. 
 *
 */

#ifndef SYSTICK_H
#define SYSTICK_H

#include "..\Type.h"
#include "..\Bit.h"

#define STK_BASE					0xE000E010

#define STK_CTRL					(STK_BASE + 0x00)
#define STK_LOAD					(STK_BASE + 0x04)
#define STK_VAL						(STK_BASE + 0x08)
#define STK_CALIB					(STK_BASE + 0x0C)

#define STK_OPT__ENA_EXCEPTION			BITHEX_1
#define STK_OPT__USE_PROC_CLK_SRC		BITHEX_2

#define STK_ENABLE(StkOptions)		IO_MEM32(STK_CTRL) = (BITHEX_0 | (StkOptions))
#define STK_DISABLE()				CLR_IO_BIT(STK_CTRL, BIT_0)
#define STK_IS_ENABLED()			GET_IO_BIT(STK_CTRL, BIT_0)
#define STK_GET_COUNT_FLAG()		GET_IO_BIT(STK_CTRL, BIT_16)

#define STK_SET_RELOAD(Reload)		IO_MEM32(STK_LOAD) = ((Reload & BITMASK_24))
#define STK_GET_VAL()				(IO_MEM32(STK_VAL))
#define STK_SET_VAL(Val)			IO_MEM32(STK_VAL) = ((Val & BITMASK_24))
#define STK_GET_CALIB()				(IO_MEM32(STK_CALIB))

#endif  // End of SYSTICK_H
