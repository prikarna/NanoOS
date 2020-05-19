/*
 * File    : Gpio.h
 * Remark  : Header of Gpio class which provide access to GPIO controller.
 *
 */

#pragma once

#include "Base.h"

class Gpio
{
public:
	enum Pin {
		Pin0,
		Pin1,
		Pin2,
		Pin3,
		Pin4,
		Pin5,
		Pin6,
		Pin7,
		Pin8,
		Pin9,
		Pin10,
		Pin11,
		Pin12,
		Pin13,
		Pin14,
		Pin15
	};

	enum InputCfg {
		InCfg_Analog = 0x00,
		InCfg_Floating = 0x04,
		InCfg_PullUpPullDown = 0x08
	};

	enum OutputCfg {
		OutCfg_GenPushPull = 0x00,
		OutCfg_GenOpenDrain = 0x04,
		OutCfg_AltPushPull = 0x08,
		OutCfg_AltOpenDrain = 0x0C
	};

	enum OutputMode {
		OutMod_10MHz = 0x01,
		OutMod_2MHz = 0x02,
		OutMod_50MHz = 0x03
	};

	enum PinGroup {
		SinglePin = 1,
		FourPins = 4,
		EightPins = 8
	};

	bool SetAsInput(Gpio::Pin StartPin, Gpio::InputCfg Cfg, Gpio::PinGroup NumbOfPin);
	bool SetAsInput(Gpio::Pin Pin, Gpio::InputCfg Cfg);
	bool SetAsOutput(Gpio::Pin StartPin, Gpio::OutputCfg Cfg, Gpio::OutputMode Mode, Gpio::PinGroup PinGroup);
	bool SetAsOutput(Gpio::Pin Pin, Gpio::OutputCfg Cfg, Gpio::OutputMode Mode);
	bool Read(Gpio::Pin StartPin, Gpio::PinGroup PinGroup, unsigned char &uRead);
	bool Read(Gpio::Pin Pin, unsigned char &uRead);
	bool Write(Gpio::Pin StartPin, Gpio::PinGroup PinGroup, unsigned char ucValue);
	bool Write(Gpio::Pin Pin, unsigned char ucValue);

protected:
	unsigned int	m_uiPortAddr;
};

class GpioA: public Base, public Gpio
{
public:
	GpioA();
	~GpioA() {};

	void *operator new(unsigned int uiSize);

private:
	static bool	m_bInit;
};

class GpioB: public Base, public Gpio
{
public:
	GpioB();
	~GpioB() {};

	void *operator new(unsigned int uiSize);

private:
	static bool	m_bInit;
};

class GpioC: public Base, public Gpio
{
public:
	GpioC();
	~GpioC() {};

	void *operator new(unsigned int uiSize);

private:
	static bool	m_bInit;
};

class GpioD: public Base, public Gpio
{
public:
	GpioD();
	~GpioD() {};

	void *operator new(unsigned int uiSize);

private:
	static bool	m_bInit;
};