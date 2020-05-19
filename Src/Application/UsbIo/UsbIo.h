/*
 * File    : UsbIo.h
 * Remark  : General definition to exchange data between UsbIoCtrl (PC progam) and UsbIo (NanoOS application)
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push)
#pragma pack(1)

#define MAX_IO_DATA		48

/*
 * GPIO
 */

#define GPIOPORT_A		0
#define GPIOPORT_B		1
#define GPIOPORT_C		2
#define GPIOPORT_D		3

#define GPIOPIN_0		0
#define GPIOPIN_1		1
#define GPIOPIN_2		2
#define GPIOPIN_3		3
#define GPIOPIN_4		4
#define GPIOPIN_5		5
#define GPIOPIN_6		6
#define GPIOPIN_7		7
#define GPIOPIN_8		8
#define GPIOPIN_9		9
#define GPIOPIN_10		10
#define GPIOPIN_11		11
#define GPIOPIN_12		12
#define GPIOPIN_13		13
#define GPIOPIN_14		14
#define GPIOPIN_15		15

#define GPIOGRP_SINGLE_PIN				0x1
#define GPIOGRP_FOUR_PINS				0x4
#define GPIOGRP_EIGHT_PINS				0x8

#define GPIOINCFG_ANALOG				0x0
#define GPIOINCFG_FLOATING				0x4
#define GPIOINCFG_PULL_UP_PULL_DOWN		0x8

#define GPIOOUTCFG_GEN_PUSH_PULL		0x0
#define GPIOOUTCFG_GEN_OPEN_DRAIN		0x4
#define GPIOOUTCFG_ALT_PUSH_PULL		0x8
#define GPIOOUTCFG_ALT_OPEN_DRAIN		0xC

#define GPIOOUTMOD_10_MHZ				0x1
#define GPIOOUTMOD_2_MHZ				0x2
#define GPIOOUTMOD_50_MHZ				0x3

typedef struct _GPIO_ADDR_DATA
{
	unsigned char	Port;
	unsigned char	Pin;
	unsigned char	PinGroup;
} GPIO_ADDR_DATA, *PGPIO_ADDR_DATA;

#define GPIOSETASTYPE_INPUT			0
#define GPIOSETASTYPE_OUPUT			1

typedef struct _GPIO_SET_AS_DATA
{
	unsigned char	Type;
	unsigned char	InputConfig;
	unsigned char	OutputConfig;
	unsigned char	OutputMode;
} GPIO_SET_AS_DATA, *PGPIO_SET_AS_DATA;

typedef struct _GPIO_WRITE_DATA
{
	unsigned char	Data;
} GPIO_WRITE_DATA, *PGPIO_WRITE_DATA;

#define GPIOCMD_SET_AS		0
#define GPIOCMD_WRITE		1
#define GPIOCMD_READ		2

typedef struct _GPIO_COMMAND
{
	unsigned char		Cmd;
	GPIO_ADDR_DATA		Address;
	union {
		GPIO_SET_AS_DATA	SetAs;
		GPIO_WRITE_DATA		Write;
	} u;
} GPIO_COMMAND, *PGPIO_COMMAND;


/*
 * USART
 */

#define USARTADDR_1				0
#define USARTADDR_2				1

#define USARTBAUD_NONE			0
#define USARTBAUD_2400			2400
#define USARTBAUD_9600			9600
#define USARTBAUD_19200			19200
#define USARTBAUD_57600			57600
#define USARTBAUD_115200		115200
#define USARTBAUD_230400		230400
#define USARTBAUD_460800		460800
#define USARTBAUD_921600		921600
#define USARTBAUD_2250000		2250000
#define USARTBAUD_4500000		4500000

#define USARTDATBITS_8			0
#define USARTDATBITS_9			1

#define USARTPARITY_NONE		0
#define USARTPARITY_ODD			1
#define USARTPARITY_EVEN		2

#define USARTSTOPBIT_1			0
#define USARTSTOPBIT_0DOT5		1
#define USARTSTOPBIT_1DOT5		2
#define USARTSTOPBIT_2			3

#define USARTFLOWCONTROL_NONE		0
#define USARTFLOWCONTROL_HARDWARE	1

typedef struct _USART_OPEN_DATA
{
	unsigned int	BaudRate;
	unsigned char	DataBits;
	unsigned char	Parity;
	unsigned char	StopBit;
	unsigned char	FlowControl;
} USART_OPEN_DATA, *PUSART_OPEN_DATA;

typedef struct _USART_WRITE_DATA
{
	unsigned char	Data[MAX_IO_DATA];
	unsigned char	Length;
} USART_WRITE_DATA, *PUSART_WRITE_DATA;

typedef struct _USART_READ_DATA
{
	unsigned char	Length;
} USART_READ_DATA, *PUSART_READ_DATA;

#define USARTCMD_OPEN		0
#define USARTCMD_WRITE		1
#define USARTCMD_READ		2
#define USARTCMD_CLOSE		3

typedef struct _USART_COMMAND
{
	unsigned char		Cmd;
	unsigned char		Address;
	union {
		USART_OPEN_DATA		Open;
		USART_WRITE_DATA	Write;
		USART_READ_DATA		Read;
	} u;
} USART_COMMAND, *PUSART_COMMAND;

/*
 * IO Request
 */

#define IOTYPE_NONE		0
#define IOTYPE_GPIO		1
#define IOTYPE_USART		2

typedef struct _IO_REQUEST {
	unsigned char	Type;
	union _REQUEST {
		GPIO_COMMAND	Gpio;
		USART_COMMAND	Usart;
	} Request;
} IO_REQUEST, *PIO_REQUEST;

#define IORESP_OK						1
#define IORESP_ERROR					2
#define IORESP_UNKNOWN_IO_REQ			11
#define IORESP_INVALID_GPIO_CMD			12
#define IORESP_INVALID_GPIO_PARAM		13
#define IORESP_GPIO_OPERATION_ERROR		14
#define IORESP_INVALID_USART_CMD		15
#define IORESP_INVALID_USART_PARAM		16
#define IORESP_USART_OPERATION_ERROR	17

typedef struct _IO_RESPONSE {
	unsigned char	Result;
	unsigned char	Length;
	unsigned char	Data[MAX_IO_DATA];
} IO_RESPONSE, *PIO_RESPONSE;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif
