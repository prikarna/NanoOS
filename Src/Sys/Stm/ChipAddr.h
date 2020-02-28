/*
 * File    : ChipAddr.h
 * Remark  : List of I/O controller address available on the chip.
 *           For now this is the list of STM32F10XXX
 *
 */

#ifndef CHIPADDR_H
#define CHIPADDR_H

#define FSMC_ADDRESS		0xA0000000	// AHB - FSMC
#define USB_OTG_ADDRESS		0x50000000	// AHB - USB OTG FS
#define ETHERNET_ADDRESS	0x40028000	// AHB - Ethernet
#define CRC_ADDRESS			0x40023000	// AHB - CRC Section
#define FLASH_CTRL_ADRESS	0x40022000	// AHB - Flash memory interface
#define RCC_ADDRESS			0x40021000	// AHB - Reset and clock control RCC
#define DMA2_ADDRESS		0x40020400	// AHB - DMA2
#define DMA1_ADDRESS		0x40020000	// AHB - DMA1
#define SDIO_ADDRESS		0x40018000	// AHB - SDIO

#define TIM11_ADDRESS		0x40015400    // APB2 - TIM11 timer
#define TIM10_ADDRESS		0x40015000    // APB2 - TIM10 timer
#define TIM9_ADDRESS		0x40014C00    // APB2 - TIM9 timer
#define ADC3_ADDRESS		0x40013C00    // APB2 - ADC3
#define USART1_ADDRESS		0x40013800    // APB2 - USART1
#define TIM8_ADDRESS		0x40013400    // APB2 - TIM8
#define SPI1_ADDRESS		0x40013000    // APB2 - SPI1
#define TIM1_ADDRESS		0x40012C00    // APB2 - TIM1
#define ADC2_ADDRESS		0x40012800    // APB2 - ADC2
#define ADC1_ADDRESS		0x40012400    // APB2 - ADC1
#define GPIO_G_ADDRESS		0x40012000    // APB2 - GPIO Port G
#define GPIO_F_ADDRESS		0x40011C00    // APB2 - GPIO Port F
#define GPIO_E_ADDRESS		0x40011800    // APB2 - GPIO Port E
#define GPIO_D_ADDRESS		0x40011400    // APB2 - GPIO Port D
#define GPIO_C_ADDRESS		0x40011000    // APB2 - GPIO Port C
#define GPIO_B_ADDRESS		0x40010C00    // APB2 - GPIO Port B
#define GPIO_A_ADDRESS		0x40010800    // APB2 - GPIO Port A
#define EXTI_ADDRESS		0x40010400    // APB2 - EXTI
#define AFIO_ADDRESS		0x40010000    // APB2 - AFIO

#define DAC_ADDRESS				0x40007400    // APB1 - DAC
#define PWR_ADDRESS				0x40007000    // APB1 - Power control PWR
#define BKP_ADDRESS				0x40006C00    // APB1 - Backup registers (BKP)
#define CAN1_ADDRESS			0x40006400    // APB1 - bxCAN1
#define CAN2_ADDRESS			0x40006800    // APB1 - bxCAN2
#define USB_OR_CAN_SRAM_ADDRESS	0x40006000    // APB1 - Shared USB/CAN SRAM 512 bytes
#define USB_FS_ADDRESS			0x40005C00    // APB1 - USB FS registers
#define I2C2_ADDRESS			0x40005800    // APB1 - I2C2
#define I2C1_ADDRESS			0x40005400    // APB1 - I2C1
#define UART5_ADDRESS			0x40005000    // APB1 - UART5
#define UART4_ADDRESS			0x40004C00    // APB1 - UART4
#define USART3_ADDRESS			0x40004800    // APB1 - USART3
#define USART2_ADDRESS			0x40004400    // APB1 - USART2
#define SPI3_OR_I2S_ADDRESS		0x40003C00    // APB1 - SPI3/I2S
#define SPI2_OR_I2S_ADDRESS		0x40003800    // APB1 - SPI2/I2S
#define IWDG_ADDRESS			0x40003000    // APB1 - Independent watchdog (IWDG)
#define WINDOW_WATCHDOG_ADDRESS	0x40002C00    // APB1 - Window watchdog (WWDG)
#define RTC_ADDRESS				0x40002800    // APB1 - RTC
#define TIM14_ADDRESS			0x40002000    // APB1 - TIM14
#define TIM13_ADDRESS			0x40001C00    // APB1 - TIM13
#define TIM12_ADDRESS			0x40001800    // APB1 - TIM12
#define TIM7_ADDRESS			0x40001400    // APB1 - TIM7
#define TIM6_ADDRESS			0x40001000    // APB1 - TIM6
#define TIM5_ADDRESS			0x40000C00    // APB1 - TIM5
#define TIM4_ADDRESS			0x40000800    // APB1 - TIM4
#define TIM3_ADSRESS			0x40000400    // APB1 - TIM3
#define TIM2_ADDRESS			0x40000000    // APB1 - TIM2

#endif  // End of CHIPADDR_H
