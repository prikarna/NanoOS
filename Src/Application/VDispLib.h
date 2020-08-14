/*
 * File    : VDispLib.h
 * Remark  : List of data definitions and functions to access VDisplay.
 *           (This lib. has been replaced by NDisplay class, provided here for hystorical
 *            purpose only and will be removed in the future).
 *
 */

#ifndef VDISPLIB_H
#define VDISPLIB_H

#include "NanoOSApi.h"
#include "VDispData.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push)
#pragma pack(1)

typedef struct _VDISP_POINT	{
	int	X;
	int	Y;
} VDISP_POINT, *PVDISP_POINT;

#define VDISP__POSITIVE_DELTA_X		0x01
#define VDISP__NEGATIVE_DELTA_X		0x02
#define VDISP__POSITIVE_DELTA_Y		0x04
#define VDISP__NEGATIVE_DELTA_Y		0x08

#define VDISP__QUADRAN_0			(VDISP__POSITIVE_DELTA_X | VDISP__POSITIVE_DELTA_Y)
#define VDISP__QUADRAN_1			(VDISP__NEGATIVE_DELTA_X | VDISP__POSITIVE_DELTA_Y)
#define VDISP__QUADRAN_2			(VDISP__NEGATIVE_DELTA_X | VDISP__NEGATIVE_DELTA_Y)
#define VDISP__QUADRAN_3			(VDISP__POSITIVE_DELTA_X | VDISP__NEGATIVE_DELTA_Y)

BOOL SetRectangle(PVDISP_RECTANGLE pRect, UINT32_T uLeft, UINT32_T uTop, UINT32_T uRight, UINT32_T uBottom);
BOOL SetPixel(UINT32_T uiX, UINT32_T uiY, UINT8_T uRed, UINT8_T uGreen, UINT8_T uBlue);
BOOL FillRectangle(
				   UINT32_T uLeft, 
				   UINT32_T uTop, 
				   UINT32_T uRight, 
				   UINT32_T uBottom, 
				   UINT8_T uRed, 
				   UINT8_T uGreen, 
				   UINT8_T uBlue
				   );
BOOL UpdateDisplay();

BOOL DrawLine(PVDISP_POINT pFromPoint, PVDISP_POINT pToPoint, UINT8_T uRed, UINT8_T uGreen, UINT8_T uBlue);
BOOL DrawHorizontalLine(UINT32_T uLeft, UINT32_T uTop, UINT32_T uLength, UINT8_T uRed, UINT8_T uGreen, UINT8_T uBlue);
BOOL DrawVerticalLine(UINT32_T uLeft, UINT32_T uTop, UINT32_T uLength, UINT8_T uRed, UINT8_T uGreen, UINT8_T uBlue);
BOOL DrawCircle(UINT32_T uX, UINT32_T uY, UINT32_T uRadius, UINT8_T uRed, UINT8_T uGreen, UINT8_T uBlue);
BOOL DrawRect(UINT32_T uLeft, UINT32_T uTop, UINT32_T uRight, UINT32_T uBottom, UINT8_T uRed, UINT8_T uGreen, UINT8_T uBlue);

BOOL GetInput(PVDISP_IN_DATA pInput);

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif  // End of VDISPLIB_H
