/*
 * File    : Interrupt.h
 * Remark  : List of interrupt number and the definition of interrupt callback function
 *
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

#define INT_TYPE__WATCHDOG							0x00	/* 16. ISR 16, Irq 0	*/
#define INT_TYPE__PVD								0x01	/* 17. ISR 17, Irq 1	*/
#define INT_TYPE__TAMPER							0x02	/* 18. ISR 18, Irq 2	*/
#define INT_TYPE__RTC								0x03	/* 19. ISR 19, Irq 3	*/
#define INT_TYPE__FLASH								0x04	/* 20. ISR 20, Irq 4	*/
#define INT_TYPE__RCC								0x05	/* 21. ISR 21, Irq 5	*/
#define INT_TYPE__EXTI0								0x06	/* 22. ISR 22, Irq 6	*/
#define INT_TYPE__EXTI1								0x07	/* 23. ISR 23, Irq 7	*/
#define INT_TYPE__EXTI2								0x08	/* 24. ISR 24, Irq 8	*/
#define INT_TYPE__EXTI3								0x09	/* 25. ISR 25, Irq 9	*/
#define INT_TYPE__EXTI4								0x0A	/* 26. ISR 26, Irq 10	*/
#define INT_TYPE__DMA1_CHAN1						0x0B	/* 27. ISR 27, Irq 11	*/
#define INT_TYPE__DMA1_CHAN2						0x0C	/* 28. ISR 28, Irq 12	*/
#define INT_TYPE__DMA1_CHAN3						0x0D	/* 29. ISR 29, Irq 13	*/
#define INT_TYPE__DMA1_CHAN4						0x0E	/* 30. ISR 30, Irq 14	*/
#define INT_TYPE__DMA1_CHAN5						0x0F	/* 31. ISR 31, Irq 15	*/
#define INT_TYPE__DMA1_CHAN6						0x10	/* 32. ISR 32, Irq 16	*/
#define INT_TYPE__DMA1_CHAN7						0x11	/* 33. ISR 33, Irq 17	*/
#define INT_TYPE__ADC1_1TO2							0x12	/* 34. ISR 34, Irq 18	*/
#define INT_TYPE__USB_HIGH_PRIO_OR_CAN1_TX			0x13	/* 35. ISR 35, Irq 19	*/
#define INT_TYPE__USB_LOW_PRIO_OR_CAN1_RX0			0x14	/* 36. ISR 36, Irq 20	*/
#define INT_TYPE__CAN1_RX1							0x15	/* 37. ISR 37, Irq 21	*/
#define INT_TYPE__CAN1_SCE							0x16    /* 38. ISR 38, Irq 22	*/
#define INT_TYPE__EXTI_9TO5							0x17    /* 39. ISR 39, Irq 23	*/
#define INT_TYPE__TIMER1_BREAK						0x18    /* 40. ISR 40, Irq 24	*/
#define INT_TYPE__TIMER1_UPDATE						0x19    /* 41. ISR 41, Irq 25	*/
#define INT_TYPE__TIMER1_TRIGGER_AND_COMMUTATION	0x1A	/* 42. ISR 42, Irq 26	*/
#define INT_TYPE__TIMER1_CAPTURE_COMPARE			0x1B	/* 43. ISR 43, Irq 27	*/
#define INT_TYPE__TIMER2							0x1C	/* 44. ISR 44, Irq 28	*/
#define INT_TYPE__TIMER3							0x1D	/* 45. ISR 45, Irq 29	*/
#define INT_TYPE__TIMER4							0x1E	/* 46. ISR 46, Irq 30	*/
#define INT_TYPE__I2C1_EVENT						0x1F	/* 47. ISR 47, Irq 31	*/
#define INT_TYPE__I2C1_ERROR						0x20	/* 48. ISR 48, Irq 32	*/
#define INT_TYPE__I2C2_EVENT						0x21	/* 49. ISR 49, Irq 33	*/
#define INT_TYPE__I2C2_ERROR						0x22	/* 50. ISR 50, Irq 34	*/
#define INT_TYPE__SPI1								0x23	/* 51. ISR 51, Irq 35	*/
#define INT_TYPE__SPI2								0x24	/* 52. ISR 52, Irq 36	*/
#define INT_TYPE__USART1							0x25	/* 53. ISR 53, Irq 37	*/
#define INT_TYPE__USART2							0x26	/* 54. ISR 54, Irq 38	*/
#define INT_TYPE__USART3							0x27	/* 55. ISR 55, Irq 39	*/
#define INT_TYPE__EXTI_15TO10						0x28	/* 56. ISR 56, Irq 40	*/
#define INT_TYPE__RTC_ALARM							0x29	/* 57. ISR 57, Irq 41	*/
#define INT_TYPE__USB_WAKEUP						0x2A	/* 58. ISR 58, Irq 42	*/
#define INT_TYPE__TIMER8_BREAK						0x2B	/* 59. ISR 59, Irq 43	*/
#define INT_TYPE__TIMER8_UPDATE						0x2C	/* 60. ISR 60, Irq 44	*/
#define INT_TYPE__TIMER8_TRIGGER_AND_COMMUTATION	0x2D	/* 61. ISR 61, Irq 45	*/
#define INT_TYPE__TIMER8_CAPTURE_COMPARE			0x2E	/* 62. ISR 62, Irq 46	*/
#define INT_TYPE__ADC3								0x2F	/* 63. ISR 63, Irq 47	*/
#define INT_TYPE__FSMC								0x30	/* 63. ISR 63, Irq 48	*/
#define INT_TYPE__SDIO								0x31	/* 64. ISR 64, Irq 49	*/
#define INT_TYPE__TIMER5							0x32	/* 65. ISR 65, Irq 50	*/
#define INT_TYPE__SPI3								0x33	/* 66. ISR 66, Irq 51	*/
#define INT_TYPE__UART4								0x34	/* 67. ISR 67, Irq 52	*/
#define INT_TYPE__UART5								0x35	/* 68. ISR 68, Irq 53	*/
#define INT_TYPE__TIMER6							0x36	/* 69. ISR 69, Irq 54	*/
#define INT_TYPE__TIMER7							0x37	/* 70. ISR 70, Irq 55	*/
#define INT_TYPE__DMA2_CHAN1						0x38	/* 71. ISR 71, Irq 56	*/
#define INT_TYPE__DMA2_CHAN2						0x39	/* 72. ISR 72, Irq 57	*/
#define INT_TYPE__DMA2_CHAN3						0x3A	/* 73. ISR 73, Irq 58	*/
#define INT_TYPE__DMA2_CHAN_4AND5					0x3B	/* 74. ISR 74, Irq 59	*/
#define INT_TYPE__RESERVED60						0x3C	/* 75. ISR 75			*/

/*
 void (* INTERRUPT_HANDLER_TYPE)(UINT32_T uiInterruptType)

 Desc.:
     Interrupt handler function signature. This handler is called when a thread set interrupt handler and one 
	 or more interrupts occured. When occurred, check uiThreadType to see what interrupt has occured and do whatever 
	 it needs in this function, for example: clear coresponding interrupt controller. This handler is called in 
	 exception or interrupt level execution thus can not call NanoOS API (Note that NanoOS API do not support being 
	 called in exception or interrupt execution level). If required this handler can call function provided by NanoOS
	 directly by refering to NanoOS.h and use elf produced by NanoOS compilation as 'some short of library' in the 
	 application compilation process.
 Params. :
	 uiInterruptType
	     One of INT_TYPE__XXX above except INT_TYPE__USART3, INT_TYPE__TIMER2 and INT_TYPE__USB_LOW_PRIO_OR_CAN1_RX0. 
		 These three interrupts are handled or used internally by NanoOS.
 Return value : 
     None.
 Error code : 
     None.
 */
typedef void (* INTERRUPT_HANDLER_TYPE)(UINT32_T uiInterruptType);

#endif  // End of INTERRUPT_H
