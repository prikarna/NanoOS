/*
 * File    : Usart.cpp
 * Remark  : Usart class implemenation.
 *
 */

#include "..\NanoOSApi.h"
#include "Usart.h"
#include "..\..\Sys\Stm\Usart.h"
#include "Gpio.h"
#include "Rcc.h"

bool Usart::_GetBaudDiv(Usart::BaudRate Baud, unsigned int &uiMantisa, unsigned int &uiFraction)
{
	unsigned int	uSysClk = 0, uApbClk = 0;
	unsigned int	uAhbDiv = 0, uApbDiv = 0;
	unsigned int	uBaud = static_cast<unsigned int>(Baud);
	unsigned int	uUaDiv = 0, uMant = 0, uFract = 0;
	RCC *			pRcc = new RCC();

	switch (m_uiUsartAddr)
	{
	case USART1:
		uApbDiv = pRcc->GetAPBDivider(false);
		break;

	case USART2:
	case USART3:
		uApbDiv = pRcc->GetAPBDivider(true);
		break;

	default:
		return false;
	}

	GetSystemClockSpeed(&uSysClk);
	uAhbDiv = pRcc->GetAHBDivider();

	uApbClk = uSysClk / (uApbDiv * uAhbDiv);
	uUaDiv = 16 * uBaud;
	uMant = uApbClk / uUaDiv;
	if ((uMant == 0) || (uMant > 0xFFE))
	{
		return false;
	}

	uFract = uApbClk - (uMant * uUaDiv);
	if (uFract == 0) {
		uiMantisa = uMant;
		uiFraction = 0;
	} else {
		uFract = (uFract * 16) / uUaDiv;
		if (uFract > 0xF) {
			uiMantisa = uMant + 1;
			uiFraction = 0;
		} else {
			uiMantisa = uMant;
			uiFraction = uFract;
		}
	}
	DBG_PRINTF("%s: uMant=%d, uFract=%d\r\n", __FUNCTION__, uMant, uFract);

	return true;
}

bool Usart::Open(Usart::BaudRate Baud, Usart::DataBits DataBits, Usart::Parity Parity, Usart::StopBit StopBit, Usart::FlowControl FlowControl)
{
	unsigned int	uiMant = 0;
	unsigned int	uiFract = 0;
	bool			bRes = false;

	if (m_bOpened) return true;

	bRes = _GetBaudDiv(Baud, uiMant, uiFract);
	if (!bRes) return false;

	RCC *			pRcc = new RCC();
	GpioA *			pIoA = 0;
	GpioB *			pIoB = 0;

	switch (m_uiUsartAddr)
	{
	case USART1:
		pRcc->EnableAPBClock(true, false, RCC_APB_HS__GPIOA | RCC_APB_HS__USART1);
		pIoA = new GpioA();
		pIoA->SetAsOutput(Gpio::Pin9, Gpio::OutCfg_AltPushPull, Gpio::OutMod_50MHz, Gpio::SinglePin);
		pIoA->SetAsInput(Gpio::Pin10, Gpio::InCfg_PullUpPullDown, Gpio::SinglePin);
		break;

	case USART2:
		pRcc->EnableAPBClock(true, false, RCC_APB_HS__GPIOA);
		pRcc->EnableAPBClock(true, true, RCC_APB_LS__USART2);
		pIoA = new GpioA();
		pIoA->SetAsOutput(Gpio::Pin2, Gpio::OutCfg_AltPushPull, Gpio::OutMod_50MHz, Gpio::SinglePin);
		pIoA->SetAsInput(Gpio::Pin3, Gpio::InCfg_PullUpPullDown, Gpio::SinglePin);
		break;

	case USART3:
		pRcc->EnableAPBClock(true, false, RCC_APB_HS__GPIOB);
		pRcc->EnableAPBClock(true, true, RCC_APB_LS__USART3);
		pIoB = new GpioB();
		pIoB->SetAsOutput(Gpio::Pin10, Gpio::OutCfg_AltPushPull, Gpio::OutMod_50MHz, Gpio::SinglePin);
		pIoB->SetAsInput(Gpio::Pin11, Gpio::InCfg_PullUpPullDown, Gpio::SinglePin);
		break;

	default:
		return false;
	}

	USART_ENABLE(m_uiUsartAddr, ENABLE);
	USART_SET_BAUD_FRACT(m_uiUsartAddr, uiMant, uiFract);

	USART_ENA_9_DATABITS(m_uiUsartAddr, DataBits);

	if (Parity != Usart::Parity_None) {
		USART_ENA_PARITY(m_uiUsartAddr, TRUE);
		if (Parity == Usart::Parity_Odd) {
			USART_ENA_ODD_PARITY(m_uiUsartAddr, TRUE);
		} else {
			USART_ENA_ODD_PARITY(m_uiUsartAddr, FALSE);
		}
	} else {
		USART_ENA_PARITY(m_uiUsartAddr, FALSE);
	}

	USART_SET_STOP_BIT(m_uiUsartAddr, StopBit);

	USART_ENA_CTS(m_uiUsartAddr, FlowControl);
	USART_ENA_RTS(m_uiUsartAddr, FlowControl);

	USART_ENA_TX(m_uiUsartAddr, ENABLE);
	USART_ENA_RX(m_uiUsartAddr, ENABLE);

	m_bOpened = true;

	return true;
}

bool Usart::Read(unsigned char &ucRead)
{
	if (!m_bOpened) return false;

	while (USART_IS_RX_NOT_EMPTY(m_uiUsartAddr) == FALSE);
	ucRead = USART_READ(m_uiUsartAddr);
	return true;
}

bool Usart::Read(unsigned char *pucRead)
{
	if (!pucRead) return false;
	if (!m_bOpened) return false;

	while (USART_IS_RX_NOT_EMPTY(m_uiUsartAddr) == FALSE);
	*pucRead = USART_READ(m_uiUsartAddr);
	return true;
}

bool Usart::Write(unsigned char ucVal)
{
	if (!m_bOpened) return false;

	while (USART_IS_TX_EMPTY(m_uiUsartAddr) == FALSE);
	USART_WRITE(m_uiUsartAddr, ucVal);
	return true;
}

bool Usart::ReadAsynch(unsigned char &ucRead)
{
	if (USART_IS_RX_NOT_EMPTY(m_uiUsartAddr)) {
		ucRead = USART_READ(m_uiUsartAddr);
		return true;
	}

	return false;
}

bool Usart::WriteAsynch(unsigned char ucVal)
{
	if (USART_IS_TX_EMPTY(m_uiUsartAddr)) {
		USART_WRITE(m_uiUsartAddr, ucVal);
		return true;
	}

	return false;
}

bool Usart::Close()
{
	if (!m_bOpened) return true;

	RCC *			pRcc = new RCC();
	GpioA *			pIoA = 0;
	GpioB *			pIoB = 0;

	switch (m_uiUsartAddr)
	{
	case USART1:
		pIoA = new GpioA();
		pIoA->SetAsInput(Gpio::Pin9, Gpio::InCfg_Analog, Gpio::SinglePin);
		pIoA->SetAsInput(Gpio::Pin10, Gpio::InCfg_Analog, Gpio::SinglePin);
		pRcc->EnableAPBClock(false, false, RCC_APB_HS__USART1);
		break;

	case USART2:
		pIoA = new GpioA();
		pIoA->SetAsInput(Gpio::Pin2, Gpio::InCfg_Analog, Gpio::SinglePin);
		pIoA->SetAsInput(Gpio::Pin3, Gpio::InCfg_Analog, Gpio::SinglePin);
		pRcc->EnableAPBClock(false, true, RCC_APB_LS__USART2);
		break;

	case USART3:
		pIoB = new GpioB();
		pIoB->SetAsInput(Gpio::Pin10, Gpio::InCfg_Analog, Gpio::SinglePin);
		pIoB->SetAsInput(Gpio::Pin11, Gpio::InCfg_Analog, Gpio::SinglePin);
		pRcc->EnableAPBClock(false, true, RCC_APB_LS__USART3);
		break;

	default:
		return false;
	}

	USART_ENA_TX(m_uiUsartAddr, DISABLE);
	USART_ENA_RX(m_uiUsartAddr, DISABLE);

	USART_ENABLE(m_uiUsartAddr, DISABLE);

	m_bOpened = false;

	return true;
}

Usart1	gUsart1;

bool Usart1::m_bInit = false;

Usart1::Usart1()
{
	if (m_bInit == false) {
		m_bInit = true;

		m_uiUsartAddr = USART1;
		m_bOpened = false;
	}
}

void *Usart1::operator new(unsigned int uiSize)
{
	return reinterpret_cast<void *>(&gUsart1);
}

Usart2	gUsart2;

bool Usart2::m_bInit = false;

Usart2::Usart2()
{
	if (m_bInit == false) {
		m_bInit = true;

		m_uiUsartAddr = USART2;
		m_bOpened = false;
	}
}

void *Usart2::operator new(unsigned int uiSize)
{
	return reinterpret_cast<void *>(&gUsart2);
}

/*
 * Reserve Usart 3
 *
Usart3	gUsart3;

bool Usart3::m_bInit = false;

Usart3::Usart3()
{
	if (m_bInit == false) {
		m_bInit = true;

		m_uiUsartAddr = USART3;
		m_bOpened = false;
	}
}

void *Usart3::operator new(unsigned int uiSize)
{
	return reinterpret_cast<void *>(&gUsart3);
}
*/
