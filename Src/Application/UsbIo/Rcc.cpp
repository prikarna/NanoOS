/*
 * File    : Rcc.cpp
 * Remark  : RCC class implementations
 *
 */

#include "Rcc.h"
#include "..\..\Sys\Stm\Rcc.h"
#include "..\NanoOSApi.h"

RCC		gRCC;

bool RCC::m_bIsInit = false;

void *RCC::operator new(unsigned int uiSize)
{
	return reinterpret_cast<void *>(&gRCC);
}

RCC::RCC():
m_HSIClock(RCC_HSI_CLOCK),
m_HSEClock(RCC_HSE_CLOCK)
{
	if (!m_bIsInit) {
		m_bIsInit = true;
	}
}

void RCC::_SetEnaHSI(bool fEna)
{
	if (fEna) {
		RCC_ENA_HSI(TRUE);
	} else {
		RCC_ENA_HSI(FALSE);
	}
}

bool RCC::_GetEnaHSI()
{
	return ((RCC_GET_HSI()) ? true : false);
}

bool RCC::_GetHSIReady()
{
	return ((RCC_IS_HSI_READY()) ? true : false);
}

void RCC::_SetHSITrim(unsigned short usTrimVal)
{
	RCC_SET_HSI_TRIMM(usTrimVal);
}

unsigned short RCC::_GetHSITrim()
{
	return (unsigned short) RCC_GET_HSI_TRIM();
}

void RCC::_SetEnaHSE(bool fEnable)
{
	if (fEnable) {
		RCC_ENA_HSE(TRUE);
	} else {
		RCC_ENA_HSE(FALSE);
	}
}

bool RCC::_GetEnaHSE()
{
	return ((RCC_GET_HSE()) ? true : false);
}

bool RCC::_GetHSEReady()
{
	return ((RCC_IS_HSE_READY()) ? true : false);
}

void RCC::_SetBypassHSE(bool fEna)
{
	if (fEna) {
		RCC_ENA_HSE_BYPASS(TRUE);
	} else {
		RCC_ENA_HSE_BYPASS(FALSE);
	}
}

bool RCC::_GetBypassHSE()
{
	return ((RCC_IS_HSE_BYPASS()) ? true : false);
}

void RCC::_SetEnaPLL(bool fEna)
{
	if (fEna) {
		RCC_ENA_PLL(TRUE);
	} else {
		RCC_ENA_PLL(FALSE);
	}
}

bool RCC::_GetEnaPLL()
{
	return ((RCC_IS_PLL_ENABLED()) ? true : false);
}

bool RCC::_GetPLLReady()
{
	return ((RCC_IS_PLL_READY()) ? true : false);
}

void RCC::_SetDivHSEForPLL(bool fEna)
{
	if (fEna) {
		RCC_SET_PLLXTPRE(SET);
	} else {
		RCC_SET_PLLXTPRE(CLEAR);
	}
}

bool RCC::_GetDivHSEForPLL()
{
	bool b = ((RCC_GET_PLLXTPRE()) ? true : false);
	return b;
}

void RCC::_SetPLLMul(RCC::Em_PLLMulFactor MulFactor)
{
	RCC_SET_PLL_MUL(MulFactor);
}

RCC::Em_PLLMulFactor RCC::_GetPLLMul()
{
	RCC::Em_PLLMulFactor m = (RCC::Em_PLLMulFactor) RCC_GET_PLL_MUL();
	return m;
}

void RCC::_SetPLLSource(RCC::Em_PLLSource Src)
{
	RCC_SET_PLL_SRC(Src);
}

RCC::Em_PLLSource RCC::_GetPLLSource()
{
	RCC::Em_PLLSource s = (RCC::Em_PLLSource) RCC_GET_PLL_SRC();
	return s;
}

void RCC::_SetAPBLoSpeedPresc(RCC::Em_APBPrescaler Presc)
{
	RCC_SET_APB1_PRESCALE(Presc);
}

RCC::Em_APBPrescaler RCC::_GetAPBLoSpeedPresc()
{
	RCC::Em_APBPrescaler p = (RCC::Em_APBPrescaler) RCC_GET_APB1_PRESCALE();
	return p;
}

void RCC::_SetAPBHiSpeedPresc(RCC::Em_APBPrescaler Presc)
{
	RCC_SET_APB2_PRESCALE(Presc);
}

RCC::Em_APBPrescaler RCC::_GetAPBHiSpeedPresc()
{
	RCC::Em_APBPrescaler p = (RCC::Em_APBPrescaler) RCC_GET_APB2_PRESCALE();
	return p;
}

void RCC::_SetSysClkSrc(RCC::Em_SystemClockSource Src)
{
	RCC_SWITCH_SYSCLK(Src);
}

RCC::Em_SystemClockSource RCC::_GetSysClkSrc()
{
	RCC::Em_SystemClockSource c = (RCC::Em_SystemClockSource) RCC_GET_SYSCLK_SRC();
	return c;
}

void RCC::_SetAHBPresc(RCC::Em_AHBPrescaler Presc)
{
	RCC_SET_AHB_PRESCALE(Presc);
}

RCC::Em_AHBPrescaler RCC::_GetAHBPresc()
{
	RCC::Em_AHBPrescaler p = (RCC::Em_AHBPrescaler) RCC_GET_AHB_PRESCALE();
	return p;
}

void RCC::EnableAHBClock(bool fEnable, unsigned int uiAHBFlags)
{
	unsigned int uiEna = ((fEnable) ? TRUE : FALSE);
	RCC_ENA_AHB_CTRL_CLK(uiEna, uiAHBFlags);
}

void RCC::EnableAPBClock(bool fEnable, bool fIsLowSpeed, unsigned int uiAPBFlags)
{
	unsigned int uiEna = ((fEnable) ? TRUE : FALSE);
	if (fIsLowSpeed) {
		RCC_ENA_APB1_CTRL_CLK(uiEna, uiAPBFlags);
	} else {
		RCC_ENA_APB2_CTRL_CLK(uiEna, uiAPBFlags);
	}
}

void RCC::ResetAPBControllers(bool fIsLowSpeed, unsigned int uiAPBFlags)
{
	if (fIsLowSpeed) {
		RCC_RESET_APB1_CTRL(uiAPBFlags);
	} else {
		RCC_RESET_APB2_CTRL(uiAPBFlags);
	}
}

unsigned int RCC::GetAHBDivider()
{
	unsigned int	uRes = 1;

	Em_AHBPrescaler	Presc = _GetAHBPresc();
	switch (Presc)
	{
	case AHBPrescaler_SYSCLKDivBy2:
		uRes = 2;
		break;

	case AHBPrescaler_SYSCLKDivBy4:
		uRes = 4;
		break;

	case AHBPrescaler_SYSCLKDivBy8:
		uRes = 8;
		break;

	case AHBPrescaler_SYSCLKDivBy16:
		uRes = 16;
		break;

	case AHBPrescaler_SYSCLKDivBy64:
		uRes = 64;
		break;

	case AHBPrescaler_SYSCLKDivBy128:
		uRes = 128;
		break;

	case AHBPrescaler_SYSCLKDivBy256:
		uRes = 256;
		break;

	case AHBPrescaler_SYSCLKDivBy512:
		uRes = 512;
		break;

	default:
		break;
	}

	return uRes;
}

unsigned int RCC::GetAPBDivider(bool fIsLowSpeedAPB)
{
	Em_APBPrescaler	Presc;
	unsigned int	uRes = 1;

	if (fIsLowSpeedAPB) {
		Presc = _GetAPBLoSpeedPresc();
	} else {
		Presc = _GetAPBHiSpeedPresc();
	}

	switch (Presc)
	{
	case APBPrescaler_HCLKDivBy2:
		uRes = 2;
		break;

	case APBPrescaler_HCLKDivBy4:
		uRes = 4;
		break;

	case APBPrescaler_HCLKDivBy8:
		uRes = 8;
		break;

	case APBPrescaler_HCLKDivBy16:
		uRes = 16;
		break;

	default:
		break;
	}

	return uRes;
}