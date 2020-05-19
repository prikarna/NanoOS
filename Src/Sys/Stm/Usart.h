/*
 * File    : Usart.h
 * Remark  : Macro to access Universal Synchronous Asynchronous 
 *           Receiver Transmitter (USART) controller on chip.
 *           Fow now this is STM32F10XXX chipset.
 *
 */

#ifndef USART_H
#define USART_H

#include "..\Type.h"
#include "..\Bit.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USART1_BASE				0x40013800
#define USART2_BASE				0x40004400
#define USART3_BASE				0x40004800
#define USART4_BASE				0x40004C00
#define USART5_BASE				0x40005000

#define USART1					USART1_BASE
#define USART2					USART2_BASE
#define USART3					USART3_BASE
#define USART4					USART4_BASE
#define USART5					USART5_BASE

#define USART_SR_OFFSET			0x00
#define USART_DR_OFFSET			0x04
#define USART_BRR_OFFSET		0x08
#define USART_CR1_OFFSET		0x0C
#define USART_CR2_OFFSET		0x10
#define USART_CR3_OFFSET		0x14
#define USART_GTPR_OFFSET		0x18

#define USART_MEM(UsartNo, RegOffset)	IO_MEM32((UsartNo + RegOffset))

#define USART_IS_TX_EMPTY(UsartNo)		(USART_MEM(UsartNo, USART_SR_OFFSET) & BITHEX_7)

#define USART_IS_TX_CMPLT(UsartNo)		(USART_MEM(UsartNo, USART_SR_OFFSET) & BITHEX_6)
#define USART_CLR_TX_CMPLT(UsartNo)		USART_MEM(UsartNo, USART_SR_OFFSET) &= ~BITHEX_6

#define USART_IS_RX_NOT_EMPTY(UsartNo)	(USART_MEM(UsartNo, USART_SR_OFFSET) & BITHEX_5)
#define USART_CLR_RX_NOT_EMPTY(UsartNo)	USART_MEM(UsartNo, USART_SR_OFFSET) &= ~BITHEX_5

#define USART_READ(UsartNo)				(USART_MEM(UsartNo, USART_DR_OFFSET))

#define USART_WRITE(UsartNo, Val)		USART_MEM(UsartNo, USART_DR_OFFSET) = Val

#define USART_SET_BAUD_FRACT(UsartNo, DivMantisa, DivFraction)			\
	USART_MEM(UsartNo, USART_BRR_OFFSET) = ((DivMantisa & 0xFFF) << 4) | (DivFraction & 0xF)

#define USART_ENABLE(UsartNo, Ena)		ENABLE_IO_BIT((UsartNo + USART_CR1_OFFSET), BIT_13, Ena)

#define USART_DATA_LEN__8_DATABITS		0x0
#define USART_DATA_LEN__9_DATABITS		0x1

#define USART_SET_DATA_LEN(DatLen)		ENABLE_IO_BIT((UsartNo + USART_CR1_OFFSET), BIT_12, DatLen)
#define USART_ENA_9_DATABITS(UsartNo, Enable)		\
	ENABLE_IO_BIT((UsartNo + USART_CR1_OFFSET), BIT_12, Enable)

#define USART_ENA_PARITY(UsartNo, Enable)	\
	ENABLE_IO_BIT((UsartNo + USART_CR1_OFFSET), BIT_10, Enable)

#define USART_ENA_ODD_PARITY(UsartNo, Enable)		\
	ENABLE_IO_BIT((UsartNo + USART_CR1_OFFSET), BIT_9, Enable)

#define USART_ENA_TX_EMPTY_INTERRUPT(UsartNo, Enable)		\
	ENABLE_IO_BIT((UsartNo + USART_CR1_OFFSET), BITNO_7, Enable)

#define USART_ENA_RX_NOT_EMPTY_INTERRUPT(UsartNo, Enable)	\
	ENABLE_IO_BIT((UsartNo + USART_CR1_OFFSET), BIT_5, Enable)

#define USART_ENA_TX(UsartNo, Ena)		ENABLE_IO_BIT((UsartNo + USART_CR1_OFFSET), BIT_3, Ena)

#define USART_ENA_RX(UsartNo, Ena)		ENABLE_IO_BIT((UsartNo + USART_CR1_OFFSET), BIT_2, Ena)

#define USART_STOP_BIT__1			0x0
#define USART_STOP_BIT__0_DOT_5		0x1
#define USART_STOP_BIT__2			0x2
#define USART_STOP_BIT__1_DOT_5		0x3

#define USART_SET_STOP_BIT(UsartNo, StopBit)		\
	SET_IO_BITS((UsartNo + USART_CR2_OFFSET), BIT_12, BITMASK_2, StopBit)

#define USART_ENA_LIN_BREAK_DET_INTERRUPT(UsartNo, Enable)	\
	ENABLE_IO_BIT((UsartNo + USART_CR2_OFFSET), BIT_6, Enable)

#define USART_GET_LIN_BREAK_DET_INTERRUPT(UsartNo)			\
	GET_IO_BIT((UsartNo + USART_CR2_OFFSET), BIT_6)

#define USART_ENA_CTS_INTERRUPT(UsartNo, Enable)	\
	ENABLE_IO_BIT((UsartNo + USART_CR3_OFFSET), BIT_10, Enable)

#define USART_GET_CTS_INTERRUPT(UsartNo)			\
	GET_IO_BIT((UsartNo + USART_CR3_OFFSET), BIT_10)

#define USART_ENA_CTS(UsartNo, Enable)				\
	ENABLE_IO_BIT((UsartNo + USART_CR3_OFFSET), BIT_9, Enable)

#define USART_ENA_RTS(UsartNo, Enable)				\
	ENABLE_IO_BIT((UsartNo + USART_CR3_OFFSET), BIT_8, Enable)

#define USART_ENA_DMA_TRANSMITTER(UsartNo, Enable)	\
	ENABLE_IO_BIT((UsartNo + USART_CR3_OFFSET), BIT_7, Enable)

#define USART_ENA_DMA_RECEIVER(UsartNo, Enable)		\
	ENABLE_IO_BIT((UsartNo + USART_CR3_OFFSET), BIT_6, Enable)

#define USART_ENA_ERROR_INTERRUPT(UsartNo, Enable)	\
	ENABLE_IO_BIT((UsartNo + USART_CR3_OFFSET), BIT_0, Enable)

#define USART_GET_ERROR_INTERRUPT(UsartNo)	\
	GET_IO_BIT((UsartNo + USART_CR3_OFFSET), BIT_0)

#ifdef __cplusplus
}
#endif

#endif  // End of USART_H
