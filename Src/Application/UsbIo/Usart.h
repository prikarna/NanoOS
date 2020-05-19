/*
 * File    : Usart.h
 * Remark  : Header of Usart class which provide access USART controller.
 *
 */

#pragma once

#include "Base.h"

class Usart
{
public:
	enum BaudRate {
		Baud_None = 0,
		Baud_2400 = 2400,
		Baud_9600 = 9600,
		Baud_19200 = 19200,
		Baud_57600 = 57600,
		Baud_115200 = 115200,
		Baud_230400 = 230400,
		Baud_460800 = 460800,
		Baud_921600 = 921600,
		Baud_2250000 = 2250000,
		Baud_4500000 = 4500000
	};

	enum DataBits {
		DataBits_8,
		DataBits_9
	};

	enum Parity {
		Parity_None,
		Parity_Odd,
		Parity_Even
	};

	enum StopBit {
		StopBit_1,
		StopBit_0Dot5,
		StopBit_1Dot5,
		StopBit_2
	};

	enum FlowControl {
		FlowControl_None,
		FlowControl_Hardware
	};

	bool Open(
			Usart::BaudRate Baud, 
			Usart::DataBits DataBits, 
			Usart::Parity Parity, 
			Usart::StopBit StopBit, 
			Usart::FlowControl FlowControl
			);
	bool Read(unsigned char &ucRead);
	bool Read(unsigned char *pucRead);
	bool Write(unsigned char ucVal);
	bool ReadAsynch(unsigned char &ucRead);
	bool WriteAsynch(unsigned char ucVal);
	bool Close();

protected:
	bool			m_bOpened;
	unsigned int	m_uiUsartAddr;

	bool _GetBaudDiv(Usart::BaudRate Baud, unsigned int &uiMantisa, unsigned int &uiFraction);
};

class Usart1: public Base, public Usart
{
public:
	Usart1();
	~Usart1() {};

	void *operator new(unsigned int uiSize);

private:
	static bool	m_bInit;
};

class Usart2: public Base, public Usart
{
public:
	Usart2();
	~Usart2() {};

	void *operator new(unsigned int uiSize);

private:
	static bool	m_bInit;
};

/*
 * Reserve Usart3, because being used by NanoOSs built-in console
 *
class Usart3: public Base, public Usart
{
public:
	Usart3();
	~Usart3() {};

	void *operator new(unsigned int uiSize);

private:
	static bool	m_bInit;
};
*/