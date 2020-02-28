/*
 * File    : UsbDesc.c
 * Remark  : USB CDC (or close to) description. 
 *
 */

#include "Type.h"
#include "UsbData.h"
#include "UsbSerial.h"

const UINT8_T	gDevDesc[] =
{
	/*
	 * USB Device descriptor
	 */
	/* bLength				*/	0x12,
	/* bDescriptorType		*/	DESCTYPE__DEVICE,
	/* bcdUSB				*/	0x00, 0x02,
	/* bDeviceClass			*/	0x02,		/*** Communication device class (CDC) ***/
	/* bDeviceSubClass		*/	0x00,
	/* bDeviceProtocol		*/	0x00,
	/* bMaxPacketSize		*/	0x40,
	/* idVendor				*/	USBSER_LVID, USBSER_HVID,	/* VID: 6A16	*/
	/* idProduct			*/	USBSER_LPID, USBSER_HPID,	/* PID: 0230	*/
	/* bcdDevice			*/	0x01, 0x01,
	/* iManufacturer		*/	0x01,
	/* iProduct				*/	0x02,
	/* iSerialNumber		*/	0x03,
	/* bNumConfigurations	*/	0x01
};

const UINT8_T	gCfgDesc[] =
{
	/*
	 * Configuration descriptor
	 * Total configuration length = 0x9(cfg) + 0x9(if ctl) + 0x7(ep1) + 0x9(if dat) + 0x7(ep2) + 0x7(ep2)
	 *							 = 0x30
	 */
	/* bLength					*/	0x09,
	/* bDescriptorType			*/	DESCTYPE__CONFIGURATION,
	/* wTotalLength				*/	0x30, 0x00,
	/* bNumInterfaces			*/	0x02,
	/* bConfigurationValue		*/	0x01,
	/* iConfiguration			*/	0x00,
	/* bmAttributes				*/	0xA0,
	/* bMaxPower				*/	0x32,

	/*
	 * Interface descriptor 0 (Communication interface)
	 */
	/* bLength					*/	0x09,
	/* bDescriptorType			*/	DESCTYPE__INTERFACE,
	/* bInterfaceNumber			*/	0x00,
	/* bAlternateSetting		*/	0x00,
	/* bNumEndPoints			*/	0x01,
	/* bInterfaceClass			*/	0x02,	/*** Communication interface class ***/
	/* bInterfaceSubClass		*/	0xFD,	/*** Vendor specific interface sub class ***/
	/* bInterfaceProtocol		*/	0x00,	/*** No interface protocol required ***/
	/* iInterface				*/	0x00,

	/*
	 * EndPoint descriptor 1
	 */
	/* bLength				*/	0x07,
	/* bDescriptorType		*/	DESCTYPE__ENDPOINT,
	/* bEndPointAddress		*/	0x81,		/* 10000001B -> EndPoint = 1, Dir. = IN */
	/* bmAttributes			*/	0x03,		/* Interrupt transfer type */
	/* wMaxPacketSize		*/	0x08, 0x00,	/* 8 bytes */
	/* bInterval			*/	0x01,

	/*
	 * Interface descriptor 1 (Data transmission interface)
	 */
	/* bLength					*/	0x09,
	/* bDescriptorType			*/	DESCTYPE__INTERFACE,
	/* bInterfaceNumber			*/	0x01,
	/* bAlternateSetting		*/	0x00,
	/* bNumEndPoints			*/	0x02,
	/* bInterfaceClass			*/	0x0A,	/*** Data interface class ***/
	/* bInterfaceSubClass		*/	0x00,	/*** Default value ***/
	/* bInterfaceProtocol		*/	0x00,	/*** No interface protocol ***/
	/* iInterface				*/	0x00,

	/*
	 * EndPoint descriptor 2
	 */
	/* bLength				*/	0x07,
	/* bDescriptorType		*/	DESCTYPE__ENDPOINT,
	/* bEndPointAddress		*/	0x82,							/* 10000010B -> EndPoint = 2, Dir. = IN */
	/* bmAttributes			*/	0x02,							/* Bulk transfer type */
	/* wMaxPacketSize		*/	USB_LDATA_SIZE, USB_HDATA_SIZE,	/* 64 bytes */
	/* bInterval			*/	0x00,

	/*
	 * EndPoint descriptor 3
	 */
	/* bLength				*/	0x07,
	/* bDescriptorType		*/	DESCTYPE__ENDPOINT,
	/* bEndPointAddress		*/	0x03,							/* 00000011B -> EndPoint = 3, Dir. = OUT */
	/* bmAttributes			*/	0x02,							/* Bulk transfer type */
	/* wMaxPacketSize		*/	USB_LDATA_SIZE, USB_HDATA_SIZE,	/* 64 bytes */
	/* bInterval			*/	0x00
};

const UINT8_T gLangId[] =
{
	/*
	 * String lang. ID
	 */
	/* bLength		*/	0x04,
	/* bDescType	*/	DESCTYPE__STRING,
	/* bString		*/	0x09, 0x04
};

const UINT8_T gMfcStr[] =
{
	/*
	 * String manufacturer
     */
	/* bLength		*/	16,
	/* bDescType	*/	DESCTYPE__STRING,
	/* bString		*/	'M', 0, 'a', 0, 'p', 0, 'a', 0, 'g', 0, 'a', 0, 'n', 0
};

const UINT8_T gProdStr[] =
{
	/*
	 * String product
	 */
	/* bLength		*/	38,
	/* bDescType	*/	DESCTYPE__STRING,
	/* bString		*/	'N', 0, 'a', 0, 'n', 0, 'o', 0, 'O', 0, 'S', 0, ' ', 0,
						'S', 0, 'e', 0, 'r', 0, 'i', 0, 'a', 0, 'l', 0, ' ', 0, 
						'P', 0, 'o', 0, 'r', 0, 't', 0
};

const UINT8_T gSerStr[] =
{
	/*
	 * String serial
	 */
	/* bLength		*/	18,
	/* bDescType	*/	DESCTYPE__STRING,
	/* bString		*/	'0', 0, '9', 0, '0', 0, '9', 0, '2', 0, '0', 0, '1', 0, '9', 0
};
