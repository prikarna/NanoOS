/*
 * File    : Gpio.h
 * Remark  : Macro to access General Purpose Input Output controller on chip.
 *           Fow now this is STM32F10XXX chipset.
 *
 */

#ifndef GPIO_H
#define GPIO_H

#include "..\Type.h"
#include "..\Bit.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GPIO_PORT_OFFSET				0x400

#define GPIO_PORT__A				0
#define GPIO_PORT__B				1
#define GPIO_PORT__C				2
#define GPIO_PORT__D				3
#define GPIO_PORT__E				4
#define GPIO_PORT__F				5
#define GPIO_PORT__G				6

#define GPIO_PORT_A_BASE				0x40010800

#define GPIO_CRL_OFFSET					0x00
#define GPIO_CRH_OFFSET					0x04
#define GPIO_IDR_OFFSET					0x08
#define GPIO_ODR_OFFSET					0x0C
#define GPIO_BSRR_OFFSET				0x10
#define GPIO_BRR_OFFSET					0x14
#define GPIO_LCKR_OFFSET				0x18

#define GPIO_PIN__0						0
#define GPIO_PIN__1						1
#define GPIO_PIN__2						2
#define GPIO_PIN__3						3
#define GPIO_PIN__4						4
#define GPIO_PIN__5						5
#define GPIO_PIN__6						6
#define GPIO_PIN__7						7
#define GPIO_PIN__8						8
#define GPIO_PIN__9						9
#define GPIO_PIN__10					10
#define GPIO_PIN__11					11
#define GPIO_PIN__12					12
#define GPIO_PIN__13					13
#define GPIO_PIN__14					14
#define GPIO_PIN__15					15
	
#define GPIO_ADDR(Port, RegOffset)		((GPIO_PORT_A_BASE + (Port * GPIO_PORT_OFFSET)) + RegOffset)

#define GPIO_CNF_INPUT__FLOATING			0x04
#define GPIO_CNF_INPUT__PULL_UP_PULL_DOWN	0x08
#define GPIO_CNF_INPUT__RESVD				0x0C

#define GPIO_CNF_OUTPUT__GEN_PUSH_PULL		0x00
#define GPIO_CNF_OUTPUT__GEN_OPEN_DRAIN		0x04
#define GPIO_CNF_OUTPUT__ALT_PUSH_PULL		0x08
#define GPIO_CNF_OUTPUT__ALT_OPEN_DRAIN		0x0C

#define GPIO_MODE__INPUT					0x00
#define GPIO_MODE__OUTPUT_10MHZ				0x01
#define GPIO_MODE__OUTPUT_2MHZ				0x02
#define GPIO_MODE__OUTPUT_50MHZ				0x03

#define GPIO_MODE_REG(Port, Pin)		(GPIO_ADDR(Port, GPIO_CRL_OFFSET) + ((Pin / 8) * 0x04))
#define GPIO_MODE_NTH_PIN(Pin)			((Pin * 4) - ((Pin / 8) * 32))

#define GPIO_SET_MODE_INPUT(Port, Pin, CnfInp)				\
	SET_IO_BITS(GPIO_MODE_REG(Port, Pin), GPIO_MODE_NTH_PIN(Pin), BITMASK_4, (GPIO_MODE__INPUT | CnfInp))

#define GPIO_SET_MODE_INPUT4(Port, StartPin, CnfInp)				\
	SET_IO_BITS(GPIO_MODE_REG(Port, StartPin), GPIO_MODE_NTH_PIN(StartPin), BITMASK_16, ((GPIO_MODE__INPUT | CnfInp) * 0x1111))

#define GPIO_SET_MODE_OUTPUT(Port, Pin, CnfOut, ModeOut)					\
	SET_IO_BITS(GPIO_MODE_REG(Port, Pin), GPIO_MODE_NTH_PIN(Pin), BITMASK_4, (CnfOut | ModeOut))

#define GPIO_SET_MODE_OUTPUT4(Port, StartPin, CnfOut, ModeOut)					\
	SET_IO_BITS(GPIO_MODE_REG(Port, StartPin), GPIO_MODE_NTH_PIN(StartPin), BITMASK_16, ((CnfOut | ModeOut) * 0x1111))

#define GPIO_SET_MODE_ANALOG(Port, Pin)						\
	SET_IO_BITS(GPIO_MODE_REG(Port, Pin), GPIO_MODE_NTH_PIN(Pin), BITMASK_4, 0x0)

#define GPIO_SET_MODE_ANALOG4(Port, StartPin)				\
	SET_IO_BITS(GPIO_MODE_REG(Port, StartPin), GPIO_MODE_NTH_PIN(StartPin), BITMASK_16, 0x0)

#define GPIO_READ(Port, Pin)				GET_IO_BIT(GPIO_ADDR(Port, GPIO_IDR_OFFSET), Pin)
#define GPIO_READ4(Port, StartPin)			GET_IO_BITS(GPIO_ADDR(Port, GPIO_IDR_OFFSET), StartPin, BITMASK_4)
#define GPIO_READ8(Port, StartPin)			GET_IO_BITS(GPIO_ADDR(Port, GPIO_IDR_OFFSET), StartPin, BITMASK_8)

#define GPIO_READ_PORT(Port)				*((UINT16_PTR_T) (Port + GPIO_IDR_OFFSET))

#define GPIO_WRITE(Port, Pin, Val)			\
	ENABLE_IO_BIT(GPIO_ADDR(Port, GPIO_ODR_OFFSET), Pin, Val)

#define GPIO_WRITE4(Port, StartPin, Val)	SET_IO_BITS(GPIO_ADDR(Port, GPIO_ODR_OFFSET), StartPin, BITMASK_4, Val)
#define GPIO_WRITE8(Port, StartPin, Val)	SET_IO_BITS(GPIO_ADDR(Port, GPIO_ODR_OFFSET), StartPin, BITMASK_8, Val)

#define GPIO_WRITE_PORT(Port, Val)			*((UINT16_PTR_T) (Port + GPIO_ODR_OFFSET)) = (UINT16_T) Val

#define GPIO_SET(Port, Pin)					SET_IO_BIT(GPIO_ADDR(Port, GPIO_BSRR_OFFSET), Pin)
#define GPIO_SET4(Port, StartPin)			SET_IO_BITS(GPIO_ADDR(Port, GPIO_BSRR_OFFSET), StartPin, BITMASK_4, 0xF)
#define GPIO_SET8(Port, StartPin)			SET_IO_BITS(GPIO_ADDR(Port, GPIO_BSRR_OFFSET), StartPin, BITMASK_8, 0xFF)

#define GPIO_RESET(Port, Pin)				SET_IO_BIT(GPIO_ADDR(Port, GPIO_BSRR_OFFSET), (Pin + 16))
#define GPIO_RESET4(Port, StartPin)			SET_IO_BITS(GPIO_ADDR(Port, GPIO_BSRR_OFFSET), (StartPin + 16), BITMASK_4, 0xF)
#define GPIO_RESET8(Port, StartPin)			SET_IO_BITS(GPIO_ADDR(Port, GPIO_BSRR_OFFSET), (StartPin + 16), BITMASK_8, 0xFF)

#ifdef __cplusplus
}
#endif

#endif  // End of GPIO_H
