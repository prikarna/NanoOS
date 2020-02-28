/*
 * File    : UsbData.h
 * Remark  : Contain USB data definition to exchange data between OS and NanoOS.
 *
 */

#ifndef USB_DATA_H
#define USB_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Error.h"

#define USB_DATA_SIZE						0x40		// 64 bytes
#define USB_LDATA_SIZE						((UINT8_T) (USB_DATA_SIZE & 0xFF))
#define USB_HDATA_SIZE						((UINT8_T) ((USB_DATA_SIZE >> 8) & 0xFF))

#define USB_INSTALL_PACKET_DATA_SIZE		(USB_DATA_SIZE - 6)
//#define USB_INSTALL_PACKET_DATA_SIZE		(USB_DATA_SIZE - 4)

#define USB_DATA_TYPE__NONE					0
#define USB_DATA_TYPE__INSTALL				0xDFEF	// Signature of USB data install packet (may conflict with other data)

#pragma pack(push)
#pragma pack(1)

typedef struct _USB_INSTALL_PACKET {
	unsigned short		Length;
	unsigned short		Index;
	unsigned char		Data[USB_INSTALL_PACKET_DATA_SIZE];
} USB_INSTALL_PACKET, *PUSB_INSTALL_PACKET;

#define USB_INST_RESP__NONE						0
#define USB_INST_RESP__SUCCESS					1

typedef struct _USB_INSTALL_RESPONSE {
	unsigned short		Code;
	unsigned char		Reserved[6];
} USB_INSTALL_RESPONSE, *PUSB_INSTALL_RESPONSE;

typedef struct _USB_DATA {
	unsigned short	Type;
	union {
		USB_INSTALL_PACKET	InstallPacket;
		unsigned char		Raw[USB_DATA_SIZE - 2];
	} u;
} USB_DATA, *PUSB_DATA;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif	// End of USB_DATA_H
