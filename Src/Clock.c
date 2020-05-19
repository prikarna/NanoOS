/*
 * File    : Clock.c
 * Remark  : System clock speed functions.
 *
 */

#include "Sys\Type.h"
#include "Sys\Stm\Flash.h"
#include "Sys\Stm\Rcc.h"

#include "NanoOS.h"

UINT32_T ClkGetPLLMultiply()
{
	UINT32_T	uMul = 0, uMulType = 0;

	uMulType = RCC_GET_PLL_MUL();
	switch (uMulType)
	{
	case RCC_PLL_MUL_BY_2:
		uMul = 2;
		break;

	case RCC_PLL_MUL_BY_3:
		uMul = 3;
		break;

	case RCC_PLL_MUL_BY_4:
		uMul = 4;
		break;

	case RCC_PLL_MUL_BY_5:
		uMul = 5;
		break;

	case RCC_PLL_MUL_BY_6:
		uMul = 6;
		break;

	case RCC_PLL_MUL_BY_7:
		uMul = 7;
		break;

	case RCC_PLL_MUL_BY_8:
		uMul = 8;
		break;

	case RCC_PLL_MUL_BY_9:
		uMul = 9;
		break;

	case RCC_PLL_MUL_BY_10:
		uMul = 10;
		break;

	case RCC_PLL_MUL_BY_11:
		uMul = 11;
		break;

	case RCC_PLL_MUL_BY_12:
		uMul = 12;
		break;

	case RCC_PLL_MUL_BY_13:
		uMul = 13;
		break;

	case RCC_PLL_MUL_BY_14:
		uMul = 14;
		break;

	case RCC_PLL_MUL_BY_15:
		uMul = 15;
		break;

	case 0xE:
	case RCC_PLL_MUL_BY_16:
		uMul = 16;
		break;

	default:
		uMul = 1;
		break;
	}

	return uMul;
}

UINT32_T ClkGetAHBDivider()
{
	UINT32_T	uAhbPreScale, uDiv;

	uAhbPreScale = RCC_GET_AHB_PRESCALE();

	switch (uAhbPreScale)
	{
	case RCC_AHB_PRESCALE__SYSCLK:
		uDiv = 1;
		break;

	case RCC_AHB_PRESCALE__SYSCLK_DIV_2:
		uDiv = 2;
		break;

	case RCC_AHB_PRESCALE__SYSCLK_DIV_4:
		uDiv = 4;
		break;

	case RCC_AHB_PRESCALE__SYSCLK_DIV_8:
		uDiv = 8;
		break;

	case RCC_AHB_PRESCALE__SYSCLK_DIV_16:
		uDiv = 16;
		break;

	case RCC_AHB_PRESCALE__SYSCLK_DIV_64:
		uDiv = 64;
		break;

	case RCC_AHB_PRESCALE__SYSCLK_DIV_128:
		uDiv = 128;
		break;

	case RCC_AHB_PRESCALE__SYSCLK_DIV_256:
		uDiv = 256;
		break;

	case RCC_AHB_PRESCALE__SYSCLK_DIV_512:
		uDiv = 512;
		break;

	default:
		uDiv = 1;
		break;
	}

	return uDiv;
}

UINT32_T ClkGetAPB1Divider()
{
	UINT32_T	uApbPreScale, uDiv;

	uApbPreScale = RCC_GET_APB1_PRESCALE();
	switch (uApbPreScale)
	{
	case RCC_APB_PRESCALE__HCLK_DIV_BY_2:
		uDiv = 2;
		break;

	case RCC_APB_PRESCALE__HCLK_DIV_BY_4:
		uDiv = 4;
		break;

	case RCC_APB_PRESCALE__HCLK_DIV_BY_8:
		uDiv = 8;
		break;

	case RCC_APB_PRESCALE__HCLK_DIV_BY_16:
		uDiv = 16;
		break;

	default:
		uDiv = 1;
		break;
	}

	return uDiv;
}

/*
UINT32_T ClkGetAPB2Divider()
{
	UINT32_T	uApbPreScale, uDiv;

	uApbPreScale = RCC_GET_APB2_PRESCALE();
	switch (uApbPreScale)
	{
	case RCC_APB_PRESCALE__HCLK_DIV_BY_2:
		uDiv = 2;
		break;

	case RCC_APB_PRESCALE__HCLK_DIV_BY_4:
		uDiv = 4;
		break;

	case RCC_APB_PRESCALE__HCLK_DIV_BY_8:
		uDiv = 8;
		break;

	case RCC_APB_PRESCALE__HCLK_DIV_BY_16:
		uDiv = 16;
		break;

	default:
		uDiv = 1;
		break;
	}

	return uDiv;
}
*/

BOOL ClkSetSystemClockToMaxSpeed()
{
	UINT8_T	u = 0, uRes = 0;

	//
	// Make flash latency longer
	//
	FLASH_ENA_PREF_BUF(TRUE);
	FLASH_SET_LATENCY(FLASH_LATENCY__TWO_WAIT_STATE);

	//
	// Enable High Speed External clock
	//
	RCC_ENA_HSE_BYPASS(DISABLE);
	RCC_ENA_HSE(ENABLE);
	while (RCC_IS_HSE_READY() == 0);

	//
	// Enable PLL clock
	//
	RCC_SET_PLL_MUL(RCC_PLL_MUL_BY_9);
	RCC_SET_PLLXTPRE(CLEAR);
	RCC_SET_PLL_SRC(RCC_PLL_SRC__HSE);
	RCC_ENA_PLL(ENABLE);
	while (RCC_IS_PLL_READY() == 0);

	//
	// Adjust peripheral which is connected to APB1
	//
	RCC_SET_APB1_PRESCALE(RCC_APB_PRESCALE__HCLK_DIV_BY_2);

	//
	// Switch system clock to PLL clock as source clock
	//
	RCC_SWITCH_SYSCLK(RCC_SYSCLK__PLL);

	//
	// Check clock switching process and return 0 if failed
	//
	for (u = 0; u < 64; u++) {
		uRes = RCC_GET_SYSCLK_SRC();
		if (uRes == RCC_SYSCLK__PLL) break;
	}

	//
	// Restore PLL register and flash latency if we fail to change clock source
	//
	if (uRes != RCC_SYSCLK__PLL) {
		FLASH_SET_LATENCY(FLASH_LATENCY__ZERO_WAIT_STATE);
		RCC_SET_APB1_PRESCALE(RCC_APB_PRESCALE__HCLK_NO_DIV);
		return FALSE;
	}

	return TRUE;
}

UINT32_T ClkGetSystemClock()
{
	UINT32_T	uClkSrc = 0, uClk = 0;
	UINT32_T	uAhbDiv = 0;
	UINT32_T	uPLLSrc, uPLLMul;

	uAhbDiv = ClkGetAHBDivider();

	uClkSrc = RCC_GET_SYSCLK_SRC();

	switch (uClkSrc)
	{
	case RCC_SYSCLK__HSI:
		uClk = RCC_HSI_CLOCK / uAhbDiv;
		break;

	case RCC_SYSCLK__HSE:
		uClk = RCC_HSE_CLOCK / uAhbDiv;
		break;

	case RCC_SYSCLK__PLL:
		uPLLSrc = RCC_GET_PLL_SRC();
		uPLLMul = ClkGetPLLMultiply();
		if (uPLLSrc == RCC_PLL_SRC__HSI_DIV_2) {
			uClk = ((RCC_HSI_CLOCK / 2) * uPLLMul) / uAhbDiv;
		} else {
			uClk = (RCC_HSE_CLOCK * uPLLMul) / uAhbDiv;
		}
		break;

	default:
		uClk = RCC_HSI_CLOCK;
		break;
	}

	return uClk;
}
