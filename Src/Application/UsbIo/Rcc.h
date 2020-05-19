/*
 * File    : Rcc.h
 * Remark  : Header of RCC class which provide access to RCC (Reset and Clock Control) controller.
 *
 */

#pragma once

#include "..\..\Sys\Bit.h"
#include "Base.h"

#define RCC_APB_HS__TIM11		BITHEX_21
#define RCC_APB_HS__TIM10		BITHEX_20
#define RCC_APB_HS__TIM9		BITHEX_19
#define RCC_APB_HS__ADC3		BITHEX_15
#define RCC_APB_HS__USART1		BITHEX_14
#define RCC_APB_HS__TIM8		BITHEX_13
#define RCC_APB_HS__SPI1		BITHEX_12
#define RCC_APB_HS__TIM1		BITHEX_11
#define RCC_APB_HS__ADC2		BITHEX_10
#define RCC_APB_HS__ADC1		BITHEX_9
#define RCC_APB_HS__GPIOG		BITHEX_8
#define RCC_APB_HS__GPIOF		BITHEX_7
#define RCC_APB_HS__GPIOE		BITHEX_6
#define RCC_APB_HS__GPIOD		BITHEX_5
#define RCC_APB_HS__GPIOC		BITHEX_4
#define RCC_APB_HS__GPIOB		BITHEX_3
#define RCC_APB_HS__GPIOA		BITHEX_2
#define RCC_APB_HS__AFIO		BITHEX_0

#define RCC_APB_LS__DAC			BITHEX_29
#define RCC_APB_LS__PWR			BITHEX_28
#define RCC_APB_LS__BKP			BITHEX_27
#define RCC_APB_LS__CAN			BITHEX_25
#define RCC_APB_LS__USB			BITHEX_23
#define RCC_APB_LS__I2C2		BITHEX_22
#define RCC_APB_LS__I2C1		BITHEX_21
#define RCC_APB_LS__UART5		BITHEX_20
#define RCC_APB_LS__UART4		BITHEX_19
#define RCC_APB_LS__USART3		BITHEX_18
#define RCC_APB_LS__USART2		BITHEX_17
#define RCC_APB_LS__SPI3		BITHEX_15
#define RCC_APB_LS__SPI2		BITHEX_14
#define RCC_APB_LS__WWDG		BITHEX_11
#define RCC_APB_LS__TIM14		BITHEX_8
#define RCC_APB_LS__TIM13		BITHEX_7
#define RCC_APB_LS__TIM12		BITHEX_6
#define RCC_APB_LS__TIM7		BITHEX_5
#define RCC_APB_LS__TIM6		BITHEX_4
#define RCC_APB_LS__TIM5		BITHEX_3
#define RCC_APB_LS__TIM4		BITHEX_2
#define RCC_APB_LS__TIM3		BITHEX_1
#define RCC_APB_LS__TIM2		BITHEX_0

#define RCC_AHB__SDIOEN			BITHEX_10
#define RCC_AHB__FSMCEN			BITHEX_8
#define RCC_AHB__CRCEN			BITHEX_6
#define RCC_AHB__FLITFEN		BITHEX_4
#define RCC_AHB__SRAMEN			BITHEX_2
#define RCC_AHB__DMA2EN			BITHEX_1
#define RCC_AHB__DMA1EN			BITHEX_0

class RCC
{
public:
	enum Em_PLLMulFactor {
		PLLMul_2 = 2,
		PLLMul_3,
		PLLMul_4,
		PLLMul_5,
		PLLMul_6,
		PLLMul_7,
		PLLMul_8,
		PLLMul_9,
		PLLMul_10,
		PLLMul_11,
		PLLMul_12,
		PLLMul_13,
		PLLMul_14,
		PLLMul_15,
		PLLMul_16
	};

	enum Em_PLLSource {
		PLLSrc_HSIDivBy2,
		PLLSrc_HSE
	};

	enum Em_APBPrescaler {
		APBPrescaler_HCLKNoDiv,
		APBPrescaler_HCLKDivBy2 = 0x04,
		APBPrescaler_HCLKDivBy4 = 0x05,
		APBPrescaler_HCLKDivBy8 = 0x06,
		APBPrescaler_HCLKDivBy16 = 0x07
	};

	enum Em_AHBPrescaler {
		AHBPrescaler_SYSCLKNoDiv,
		AHBPrescaler_SYSCLKDivBy2 = 0x08,
		AHBPrescaler_SYSCLKDivBy4,
		AHBPrescaler_SYSCLKDivBy8,
		AHBPrescaler_SYSCLKDivBy16,
		AHBPrescaler_SYSCLKDivBy64,
		AHBPrescaler_SYSCLKDivBy128,
		AHBPrescaler_SYSCLKDivBy256,
		AHBPrescaler_SYSCLKDivBy512
	};

	enum Em_SystemClockSource {
		SystemClock_HSI,
		SystemClock_HSE,
		SystemClock_PLL
	};

private:
	static bool			m_bIsInit;
	void				_SetEnaHSI(bool fEnable);
	bool				_GetEnaHSI();
	bool				_GetHSIReady();
	void				_SetHSITrim(unsigned short usTrimVal);
	unsigned short		_GetHSITrim();
	void				_SetEnaHSE(bool fEnable);
	bool				_GetEnaHSE();
	bool				_GetHSEReady();
	void				_SetBypassHSE(bool fEnable);
	bool				_GetBypassHSE();
	void				_SetEnaPLL(bool fEna);
	bool				_GetEnaPLL();
	bool				_GetPLLReady();
	void				_SetDivHSEForPLL(bool fEna);
	bool				_GetDivHSEForPLL();
	void				_SetPLLMul(RCC::Em_PLLMulFactor MulFactor);
	RCC::Em_PLLMulFactor	_GetPLLMul();
	void					_SetPLLSource(RCC::Em_PLLSource Src);
	RCC::Em_PLLSource		_GetPLLSource();
	void					_SetAPBLoSpeedPresc(RCC::Em_APBPrescaler Presc);
	RCC::Em_APBPrescaler	_GetAPBLoSpeedPresc();
	void					_SetAPBHiSpeedPresc(RCC::Em_APBPrescaler Presc);
	RCC::Em_APBPrescaler	_GetAPBHiSpeedPresc();
	void					_SetSysClkSrc(RCC::Em_SystemClockSource Src);
	RCC::Em_SystemClockSource	_GetSysClkSrc();
	void					_SetAHBPresc(RCC::Em_AHBPrescaler Presc);
	RCC::Em_AHBPrescaler	_GetAHBPresc();

public:
	const unsigned int	m_HSIClock;
	const unsigned int	m_HSEClock;

	RCC();
	~RCC() {};

	void * operator new(unsigned int uiSize);

	void EnableAPBClock(bool fEnable, bool fIsLowSpeed, unsigned int uiAPBFlags);
	void ResetAPBControllers(bool fIsLowSpeed, unsigned int uiAPBFlags);
	void EnableAHBClock(bool fEnable, unsigned int uiAHBFlags);

	unsigned int GetAHBDivider();
	unsigned int GetAPBDivider(bool fIsLowSpeedAPB);
};