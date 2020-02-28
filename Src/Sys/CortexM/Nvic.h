/*
 * File    : Nvic.h
 * Remark  : Macro to access Cortex-M NVIC (Nested Vectored Interrupt Controller). 
 *
 */

#ifndef NVIC_H
#define NVIC_H

#include "..\Type.h"
#include "..\Bit.h"

#define NVIC_BASE		0xE000E100

#define NVIC_ISER0		(NVIC_BASE + 0x000)
#define NVIC_ICER0		(NVIC_BASE + 0x080)
#define NVIC_ISPR0		(NVIC_BASE + 0x100)
#define NVIC_ICPR0		(NVIC_BASE + 0x180)
#define NVIC_IABR0		(NVIC_BASE + 0x200)

#define NVIC_IPR0		(NVIC_BASE + 0x300)		// IRQ 0 - 3
#define NVIC_IPR1		(NVIC_IPR0 + 0x4)		// IRQ 4 - 7
#define NVIC_IPR2		(NVIC_IPR0 + 0x8)		// IRQ 8 - 11
#define NVIC_IPR3		(NVIC_IPR0 + 0xC)		// IRQ 12 - 15
#define NVIC_IPR4		(NVIC_IPR0 + 0x10)		// IRQ 16 - 19
#define NVIC_IPR5		(NVIC_IPR0 + 0x14)		// IRQ 20 - 23
#define NVIC_IPR6		(NVIC_IPR0 + 0x18)		// IRQ 24 - 27
#define NVIC_IPR7		(NVIC_IPR0 + 0x1C)		// IRQ 28 - 31
#define NVIC_IPR8		(NVIC_IPR0 + 0x20)		// IRQ 32 - 35
#define NVIC_IPR9		(NVIC_IPR0 + 0x24)		// IRQ 36 - 39
#define NVIC_IPR10		(NVIC_IPR0 + 0x28)		// IRQ 40 - 43
#define NVIC_IPR11		(NVIC_IPR0 + 0x2C)		// IRQ 44 - 47
#define NVIC_IPR12		(NVIC_IPR0 + 0x30)		// IRQ 48 - 51
#define NVIC_IPR13		(NVIC_IPR0 + 0x34)		// IRQ 52 - 55
#define NVIC_IPR14		(NVIC_IPR0 + 0x38)		// IRQ 56 - 59
#define NVIC_IPR15		(NVIC_IPR0 + 0x3C)		// IRQ 60 - 63
#define NVIC_IPR16		(NVIC_IPR0 + 0x40)		// IRQ 64 - 67
#define NVIC_IPR17		(NVIC_IPR0 + 0x44)		// IRQ 68 - 71
#define NVIC_IPR18		(NVIC_IPR0 + 0x48)		// IRQ 72 - 75
#define NVIC_IPR19		(NVIC_IPR0 + 0x4C)		// IRQ 76 - 79
#define NVIC_IPR20		(NVIC_IPR0 + 0x50)		// IRQ 80

#define NVIC_STIR		(NVIC_BASE + 0xE00)

#define NVIC_IRQ__WINDOW_WATCHDOG				0x00	/* 0. ISR 16, IntWindowWatchDog */
#define NVIC_IRQ__PVD							0x01	/* 1. ISR 17, IntPVD */
#define NVIC_IRQ__TAMPER						0x02	/* 2. ISR 18, IntTamper */
#define NVIC_IRQ__RTC							0x03	/* 3. ISR 19, IntRTC */
#define NVIC_IRQ__FLASH							0x04	/* 4. ISR 20, IntFlash */
#define NVIC_IRQ__RCC							0x05	/* 5. ISR 21, IntRCC */
#define NVIC_IRQ__EXTERNAL_INT0					0x06	/* 6. ISR 22, IntEXTI0 */
#define NVIC_IRQ__EXTERNAL_INT1					0x07	/* 7. ISR 23, IntEXTI1 */
#define NVIC_IRQ__EXTERNAL_INT2					0x08	/* 8. ISR 24, IntEXTI2 */
#define NVIC_IRQ__EXTERNAL_INT3					0x09	/* 9. ISR 25, IntEXTI3 */
#define NVIC_IRQ__EXTERNAL_INT4					0x0A	/* 10. ISR 26, IntEXTI4 */
#define NVIC_IRQ__DMA1_CHANNEL1					0x0B	/* 11. ISR 27, IntDMA1Channel1 */
#define NVIC_IRQ__DMA1_CHANNEL2					0x0C	/* 12. ISR 28, IntDMA1Channel2 */
#define NVIC_IRQ__DMA1_CHANNEL3					0x0D	/* 13. ISR 29, IntDMA1Channel3 */
#define NVIC_IRQ__DMA1_CHANNEL4					0x0E	/* 14. ISR 30, IntDMA1Channel4 */
#define NVIC_IRQ__DMA1_CHANNEL5					0x0F	/* 15. ISR 31, IntDMA1Channel5 */
#define NVIC_IRQ__DMA1_CHANNEL6					0x10	/* 16. ISR 32, IntDMA1Channel6 */
#define NVIC_IRQ__DMA1_CHANNEL7					0x11	/* 17. ISR 33, IntDMA1Channel7 */
#define NVIC_IRQ__ADC_1TO2						0x12	/* 18. ISR 34, IntADC1To2 */
#define NVIC_IRQ__USB_LOW_PRIO_OR_CAN1_TX		0x13	/* 19. ISR 35, IntUSBLowPriorityOrCAN1_TX */
#define NVIC_IRQ__USB_HIGH_PRIO_OR_CAN1_RX0		0x14	/* 20. ISR 36, IntUSBHighPriorityOrCAN1_RX0 */
#define NVIC_IRQ__CAN1_RX1						0x15	/* 21. ISR 37, IntCAN1_RX1 */
#define NVIC_IRQ__CAN1_SCE						0x16	/* 22. ISR 38, IntCAN1_SCE */
#define NVIC_IRQ__EXTERNAL_INT9_TO_5			0x17	/* 23. ISR 39, IntEXTI9To5 */
#define NVIC_IRQ__TIMER1_BREAK					0x18	/* 24. ISR 40, IntTimer1Break */
#define NVIC_IRQ__TIMER1_UPDATE					0x19	/* 25. ISR 41, IntTimer1Update */
#define NVIC_IRQ__TIMER1_TRIG_AND_COMMUTATION	0x1A	/* 26. ISR 42, IntTimer1TriggerAndCommutation */
#define NVIC_IRQ__TIMER1_COMPARE_CAPTURE		0x1B	/* 27. ISR 43, IntTimer1CaptureCompare */
#define NVIC_IRQ__TIMER2						0x1C	/* 28. ISR 44, IntTimer2 */
#define NVIC_IRQ__TIMER3						0x1D	/* 29. ISR 45, IntTimer3 */
#define NVIC_IRQ__TIMER4						0x1E	/* 30. ISR 46, IntTimer4 */
#define NVIC_IRQ__I2C1_EVENT					0x1F	/* 31. ISR 47, IntI2C1Event */
#define NVIC_IRQ__I2C1_ERROR					0x20	/* 32. ISR 48, IntI2C1Error */
#define NVIC_IRQ__I2C2_EVENT					0x21	/* 33. ISR 49, IntI2C2Event */
#define NVIC_IRQ__I2C2_ERROR					0x22	/* 34. ISR 50, IntI2C2Error */
#define NVIC_IRQ__SPI1							0x23	/* 35. ISR 51, IntSPI1 */
#define NVIC_IRQ__SPI2							0x24	/* 36. ISR 52, IntSPI2 */
#define NVIC_IRQ__USART1						0x25	/* 37. ISR 53, IntUSART1 */
#define NVIC_IRQ__USART2						0x26	/* 38. ISR 54, IntUSART2 */
#define NVIC_IRQ__USART3						0x27	/* 39. ISR 55, IntUSART3 */
#define NVIC_IRQ__EXTERNAL_INT15_TO_10			0x28	/* 40. ISR 56, IntEXTI15To10 */
#define NVIC_IRQ__RTC_ALARM						0x29	/* 41. ISR 57, IntRTCAlarm */
#define NVIC_IRQ__USB_WAKEUP					0x2A	/* 42. ISR 58, IntUSBWakeUp */
#define NVIC_IRQ__TIMER8_BREAK					0x2B	/* 43. ISR 59, IntTimer8Break */
#define NVIC_IRQ__TIMER8_UPDATE					0x2C	/* 44. ISR 60, IntTimer8Update */
#define NVIC_IRQ__TIMER8_TRIG_AND_COMMUTATION	0x2D	/* 45. ISR 61, IntTimer8TriggerAndCommutation */
#define NVIC_IRQ__TIMER8_COMPARE_CAPTURE		0x2E	/* 46. ISR 62, IntTimer8CaptureCompare */
#define NVIC_IRQ__ADC3							0x2F	/* 47. ISR 63, IntADC3 */
#define NVIC_IRQ__FSMC							0x30	/* 48. ISR 63, IntFSMC */
#define NVIC_IRQ__SDIO							0x31	/* 49. ISR 64, IntSDIO */
#define NVIC_IRQ__TIMER5						0x32	/* 50. ISR 65, IntTimer5 */
#define NVIC_IRQ__SPI3							0x33	/* 51. ISR 66, IntSPI3 */
#define NVIC_IRQ__UART4							0x34	/* 52. ISR 67, IntUART4 */
#define NVIC_IRQ__UART5							0x35	/* 53. ISR 68, IntUART5 */
#define NVIC_IRQ__TIMER6						0x36	/* 54. ISR 69, IntTimer6 */
#define NVIC_IRQ__TIMER7						0x37	/* 55. ISR 70, IntTimer7 */
#define NVIC_IRQ__DMA2_CHANNEL1					0x38	/* 56. ISR 71, IntDMA2Channel1 */
#define NVIC_IRQ__DMA2_CHANNEL2					0x39	/* 57. ISR 72, IntDMA2Channel2 */
#define NVIC_IRQ__DMA2_CHANNEL3					0x3A	/* 58. ISR 73, IntDMA2Channel3 */
#define NVIC_IRQ__DMA2_CHANNEL4_AND_5			0x3B	/* 59. ISR 74, IntDMA2Channel4And5 */
#define NVIC_IRQ__MAXIMUM						0x3C	/* 60. For watermark only */

#define NVIC__REG_ADDR(RegAddrBase, NvicIrq)	(RegAddrBase + ((NvicIrq / 32) * 0x4))
#define NVIC__NTH_IRQ(NvicIrq)					(NvicIrq % 32)

#define NVIC_ENA_INT(NvicIrq)						\
	SET_IO_BIT(NVIC__REG_ADDR(NVIC_ISER0, NvicIrq), NVIC__NTH_IRQ(NvicIrq))

#define NVIC_ENA_ALL_INTS()							\
	WRITE_IO_MEM32(NVIC_ISER0, BITMASK_32);			\
	WRITE_IO_MEM32((NVIC_ISER0 + 0x4), BITMASK_32);	\
	WRITE_IO_MEM32((NVIC_ISER0 + 0x8), BITMASK_16)

#define NVIC_CLR_INT(NvicIrq)						\
	SET_IO_BIT(NVIC__REG_ADDR(NVIC_ICER0, NvicIrq), NVIC__NTH_IRQ(NvicIrq))

#define NVIC_CLR_ALL_INTS()							\
	WRITE_IO_MEM32(NVIC_ICER0, BITMASK_32);			\
	WRITE_IO_MEM32((NVIC_ICER0 + 0x4), BITMASK_32);	\
	WRITE_IO_MEM32((NVIC_ICER0 + 0x8), BITMASK_16)

#define NVIC_SET_PENDING_INT(NvicIrq)			\
	SET_IO_BIT(NVIC_REG_ADDR(NVIC_ISPR0, NvicIrq), NVIC_NTH_IRQ(NvicIrq))

#define NVIC_CLR_PENDING_INT(NvicIrq)			\
	SET_IO_BIT(NVIC_REG_ADDR(NVIC_ICPR0, NvicIrq), NVIC_NTH_IRQ(NvicIrq))

#define NVIC_IS_ACTIVE_INT(NvicIrq)				\
	GET_IO_BIT(NVIC_REG_ADDR(NVIC_IABR0, NvicIrq), NVIC_NTH_IRQ(NvicIrq))

#define NVIC_PRIO__REG_ADDR(NvicIrq)			(NVIC_IPR0 + ((NvicIrq / 0x4) * 0x4))
#define NVIC_PRIO__NTH_BIT(NvicIrq)				(((NvicIrq % 0x4) * 8) + 4)

#define NVIC_GET_IRQ_PRIO(NvicIrq)						\
	GET_IO_BITS(NVIC_PRIO__REG_ADDR(NvicIrq), NVIC_PRIO__NTH_BIT(NvicIrq), BITMASK_4)

#define NVIC_SET_IRQ_PRIO(NvicIrq, NvicPrio)			\
	SET_IO_BITS(NVIC_PRIO__REG_ADDR(NvicIrq), NVIC_PRIO__NTH_BIT(NvicIrq), BITMASK_4, NvicPrio)

#define NVIC_SOFT_TRIG_INT(NvicIrq)				SET_IO_BITS(NVIC_STIR, BIT_0, BITMASK_9, NvicIrq)

#endif   // End of NVIC_H
