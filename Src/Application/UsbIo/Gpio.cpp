/*
 * File    : Gpio.cpp
 * Remark  : Implementation of Gpio class
 *
 */

#include "Gpio.h"
#include "..\..\Sys\Stm\Gpio.h"

bool Gpio::SetAsInput(Gpio::Pin StartPin, Gpio::InputCfg Cfg, Gpio::PinGroup PinGroup)
{
	switch (PinGroup)
	{
	case Gpio::SinglePin:
		GPIO_SET_MODE_INPUT(m_uiPortAddr, StartPin, Cfg);
		break;

	case Gpio::FourPins:
		GPIO_SET_MODE_INPUT4(m_uiPortAddr, StartPin, Cfg);
		break;

	default:
		return false;
	}

	return true;
}

bool Gpio::SetAsInput(Gpio::Pin Pin, Gpio::InputCfg Cfg)
{
	GPIO_SET_MODE_INPUT(m_uiPortAddr, Pin, Cfg);
	return true;
}

bool Gpio::SetAsOutput(Gpio::Pin StartPin, Gpio::OutputCfg Cfg, Gpio::OutputMode Mode, Gpio::PinGroup PinGroup)
{
	switch (PinGroup)
	{
	case Gpio::SinglePin:
		GPIO_SET_MODE_OUTPUT(m_uiPortAddr, StartPin, Cfg, Mode);
		break;

	case Gpio::FourPins:
		GPIO_SET_MODE_OUTPUT4(m_uiPortAddr, StartPin, Cfg, Mode);
		break;

	default:
		return false;
	}

	return true;
}

bool Gpio::SetAsOutput(Gpio::Pin Pin, Gpio::OutputCfg Cfg, Gpio::OutputMode Mode)
{
	GPIO_SET_MODE_OUTPUT(m_uiPortAddr, Pin, Cfg, Mode);
	return true;
}

bool Gpio::Read(Gpio::Pin StartPin, Gpio::PinGroup PinGroup, unsigned char &uRead)
{
	switch (PinGroup)
	{
	case Gpio::SinglePin:
		uRead = GPIO_READ(m_uiPortAddr, StartPin);
		break;

	case Gpio::FourPins:
		uRead = GPIO_READ4(m_uiPortAddr, StartPin);
		break;

	case Gpio::EightPins:
		uRead = GPIO_READ8(m_uiPortAddr, StartPin);
		break;

	default:
		return false;
	}

	return true;
}

bool Gpio::Read(Gpio::Pin Pin, unsigned char &uiRead)
{
	uiRead = GPIO_READ(m_uiPortAddr, Pin);
	return true;
}

bool Gpio::Write(Gpio::Pin StartPin, Gpio::PinGroup PinGroup, unsigned char ucValue)
{
	switch (PinGroup)
	{
	case Gpio::SinglePin:
		GPIO_WRITE(m_uiPortAddr, StartPin, ucValue);
		break;

	case Gpio::FourPins:
		GPIO_WRITE4(m_uiPortAddr, StartPin, ucValue);
		break;

	case Gpio::EightPins:
		GPIO_WRITE8(m_uiPortAddr, StartPin, ucValue);
		break;

	default:
		return false;
	}

	return true;
}

bool Gpio::Write(Gpio::Pin Pin, unsigned char ucValue)
{
	GPIO_WRITE(m_uiPortAddr, Pin, ucValue);
	return true;
}

GpioA	gGpioA;

bool GpioA::m_bInit = false;

GpioA::GpioA()
{
	if (!m_bInit) {
		m_bInit = true;

		m_uiPortAddr = GPIO_PORT__A;
	}
}

void *GpioA::operator new(unsigned int uiSize)
{
	return reinterpret_cast<void *>(&gGpioA);
}

GpioB	gGpioB;

bool GpioB::m_bInit = false;

GpioB::GpioB()
{
	if (!m_bInit) {
		m_bInit = true;

		m_uiPortAddr = GPIO_PORT__B;
	}
}

void *GpioB::operator new(unsigned int uiSize)
{
	return reinterpret_cast<void *>(&gGpioB);
}

GpioC	gGpioC;

bool GpioC::m_bInit = false;

GpioC::GpioC()
{
	if (!m_bInit) {
		m_bInit = true;

		m_uiPortAddr = GPIO_PORT__C;
	}
}

void *GpioC::operator new(unsigned int uiSize)
{
	return reinterpret_cast<void *>(&gGpioC);
}

GpioD	gGpioD;

bool GpioD::m_bInit = false;

GpioD::GpioD()
{
	if (!m_bInit) {
		m_bInit = true;

		m_uiPortAddr = GPIO_PORT__D;
	}
}

void *GpioD::operator new(unsigned int uiSize)
{
	return reinterpret_cast<void *>(&gGpioD);
}