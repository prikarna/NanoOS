/*
 * File    : UsbSerial.h
 * Remark  : USB serial converter data definition.
 *
 */

#ifndef USBSERIAL_H
#define USBSERIAL_H

#include "Sys\Type.h"

#define USBSER_VID						0x6A16
#define USBSER_PID						0x0230
#define USBSER_LVID						((UINT8_T) (USBSER_VID & 0xFF))
#define USBSER_HVID						((UINT8_T) ((USBSER_VID >> 8) & 0xFF))
#define USBSER_LPID						((UINT8_T) (USBSER_PID & 0xFF))
#define USBSER_HPID						((UINT8_T) ((USBSER_PID >> 8) & 0xFF))

/*
 * USB interface
 */
#define DESCTYPE__DEVICE					1
#define DESCTYPE__CONFIGURATION				2
#define DESCTYPE__STRING					3
#define DESCTYPE__INTERFACE					4
#define DESCTYPE__ENDPOINT					5
#define DESCTYPE__DEVICE_QUALIFIER			6
#define DESCTYPE__OTHER_SPEED_CONFIGURATION	7
#define DESCTYPE__INTERFACE_POWER			8
#define DESCTYPE__OTG						9
#define DESCTYPE__DEBUG						10
#define DESCTYPE__INTERFACE_ASSOCIATION		11

typedef struct _USB_DEVICE_DESCRIPTOR {
	unsigned char			bLength;
	unsigned char			bDescriptorType;
	unsigned short			bcdUSB;
	unsigned char			bDeviceClass;
	unsigned char			bDeviceSubClass;
	unsigned char			bDeviceProtocol;
	unsigned char			bMaxPacketSize0;
	unsigned short			idVendor;
	unsigned short			idProduct;
	unsigned short			bcdDevice;
	unsigned char			iManufacturer;
	unsigned char			iProduct;
	unsigned char			iSerialNumber;
	unsigned char			bNumConfigurations;
} __attribute__((packed)) USB_DEVICE_DESCRIPTOR, *PUSB_DEVICE_DESCRIPTOR;

typedef struct _USB_CONFIGURATION_DESCRIPTOR {
	unsigned char			bLength;
	unsigned char			bDescriptorType;
	unsigned short			wTotalLength;
	unsigned char			bNumInterfaces;
	unsigned char			bConfigurationValue;
	unsigned char			iConfiguration;
	unsigned char			bmAttributes;
	unsigned char			bMaxPower;
} __attribute__((packed)) USB_CONFIGURATION_DESCRIPTOR, *PUSB_CONFIGURATION_DESCRIPTOR;

#define USBSTD__CLEAR_FEATURE				0x0000
#define USBSTD__CLEAR_FEATURE_INTERFACE		0x0101
#define USBSTD__CLEAR_FEATURE_ENDPOINT		0x0201
#define USBSTD__GET_CONFIGURATION			0x8008
#define USBSTD__GET_DESCRIPTOR				0x8006
#define USBSTD__GET_DESCRIPTOR_INTERFACE	0x8106
#define USBSTD__GET_DESCRIPTOR_ENDPOINT		0x8206
#define USBSTD__GET_INTERFACE				0x810A
#define USBSTD__GET_STATUS					0x8000
#define USBSTD__GET_STATUS_INTERFACE		0x8100
#define USBSTD__GET_STATUS_ENDPOINT			0x8200
#define USBSTD__SET_ADDRESS					0x0005
#define USBSTD__SET_CONFIGURATION			0x0009
#define USBSTD__SET_DESCRIPTOR				0x0007
#define USBSTD__SET_FEATURE					0x0003
#define USBSTD__SET_FEATURE_INTERFACE		0x0102
#define USBSTD__SET_FEATURE_ENDPOINT		0x0203
#define USBSTD__SET_INTERFACE				0x010B
#define USBSTD__SYNC_FRAME					0x820C

#define USBCDC__SET_LINE_CODING				0x2120
#define USBCDC__GET_LINE_CODING				0xA121
#define USBCDC__SET_CONTROL_LINE_STATE		0x2122

struct _USB_SETUP {
	unsigned char	bmRequestType;
	unsigned char	bRequest;
	unsigned short	wValue;
	unsigned short	wIndex;
	unsigned short	wLength;
} __attribute__((packed));

typedef struct _USB_SETUP		USB_SETUP, *PUSB_SETUP;

struct _LINE_CODING	{
	unsigned int		dwDTERate;
	unsigned char		bCharFormat;
	unsigned char		bParityType;
	unsigned char		bDataBits;
}__attribute__((packed));

typedef struct _LINE_CODING		LINE_CODING, *PLINE_CODING;

#include "UsbData.h"

/*
 *							   Size of USB_BUFFER = 0x40 = 64 bytes
 */
#define USBEP0_TX_BUF_LEN		0x40					/* 64 bytes	*/
#define USBEP0_RX_BUF_LEN		0x40					/* 64 bytes	*/
#define USBEP1_BUF_LEN			0x08					/* 8 bytes	*/
#define USBEP2_BUF_LEN			USB_DATA_SIZE			/* 64 bytes	*/
#define USBEP2_RSVD_BUF_LEN		0x20					/* 32 bytes */
#define USBEP3_BUF_LEN			USB_DATA_SIZE			/* 64 bytes	*/

struct _USB_BUFFER {
	//
	// Ep 0
	//
	UINT16_T		Ep0TxOffset;
	UINT16_T		Pad0;
	UINT16_T		Ep0TxLength;
	UINT16_T		Pad1;
	UINT16_T		Ep0RxOffset;
	UINT16_T		Pad2;
	UINT16_T		Ep0RxLength;
	UINT16_T		Pad3;
	
	//
	// Ep 1
	//
	UINT16_T		Ep1TxOffset;
	UINT16_T		Pad4;
	UINT16_T		Ep1TxLength;
	UINT16_T		Pad5;
	UINT16_T		Ep1RxOffset;
	UINT16_T		Pad6;
	UINT16_T		Ep1RxLength;
	UINT16_T		Pad7;

	//
	// Ep 2
	//
	UINT16_T		Ep2TxOffset;
	UINT16_T		Pad8;
	UINT16_T		Ep2TxLength;
	UINT16_T		Pad9;
	UINT16_T		Ep2RxOffset;
	UINT16_T		Pad10;
	UINT16_T		Ep2RxLength;
	UINT16_T		Pad11;

	//
	// Ep 3
	//
	UINT16_T		Ep3TxOffset;
	UINT16_T		Pad12;
	UINT16_T		Ep3TxLength;
	UINT16_T		Pad13;
	UINT16_T		Ep3RxOffset;
	UINT16_T		Pad14;
	UINT16_T		Ep3RxLength;
	UINT16_T		Pad15;
} __attribute__((packed));

typedef struct _USB_BUFFER	USB_BUFFER, *PUSB_BUFFER;

#define SIZEOF_USB_BUFFER									sizeof(USB_BUFFER)
#define USB_RX_BUFFER_LENGTH(BlockSize, NumbOfBlock)		((((((UINT32_T) BlockSize) & 0x1) << 5) | (((UINT32_T) NumbOfBlock) & BITMASK_5)) << 10)
#define USB_GET_RX_BUFFER_COUNT(EndPointRxBufferLength)		(EndPointRxBufferLength & 0x3FF)
#define USB_GET_BUFFER_ADDRESS(EndPointBufferOffset)		(UINT8_PTR_T) (USB_BUFF_ADDR + (EndPointBufferOffset * 2))

#define USB_IO_INT_ERRORS				\
	(USB_INT__ERROR | USB_INT__PACK_MEM_OVR_UND_RUN | USB_INT__WAKEUP | USB_INT__SUSPEND | USB_INT__RESET)

#define USB_CLEAR_UNUSED_INTRS()		IO_MEM32(USB_ISTR) = 0x1C00

int UsbProgramFlash(UINT8_PTR_T pUsbRecvBuf);

#define USB_RECV__NONE			0x00
#define USB_RECV__IN_PROGRESS	0x01
#define USB_RECV__CANCELLED		0x02
#define USB_RECV__COMPLETE		0x04

BOOL UsbCompleteReceive();

#endif  // End of USBSERIAL_H
