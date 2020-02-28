/*
 * File    : VDispLib.c
 * Remark  : Implementation of VDisplay functions to access NanoOS Terminal's VDisplay component.
 *
 */

#include "NanoOSApi.h"
#include "VDispLib.h"

static VDISP_OUT_DATA		sOutDat = {0};

BOOL SetRectangle(PVDISP_RECTANGLE pRect, UINT32_T uLeft, UINT32_T uTop, UINT32_T uRight, UINT32_T uBottom)
{
	if (!pRect) return FALSE;

	pRect->Left = uLeft;
	pRect->Top = uTop;
	pRect->Right = uRight;
	pRect->Bottom = uBottom;

	return TRUE;
}

BOOL SetPixel(UINT32_T uiX, UINT32_T uiY, UINT8_T uRed, UINT8_T uGreen, UINT8_T uBlue)
{
	BOOL	fRes = FALSE;

	sOutDat.Type = VDISP_OUT_TYPE__PIXEL;
	sOutDat.u.Pixel.Color = VDISP_RGB(uRed, uGreen, uBlue);
	sOutDat.u.Pixel.X = uiX;
	sOutDat.u.Pixel.Y = uiY;

	fRes = WriteToUsbSerial((UINT8_PTR_T) &sOutDat, sizeof(VDISP_OUT_DATA));

	return fRes;
}

BOOL FillRectangle(
				   UINT32_T uLeft, 
				   UINT32_T uTop, 
				   UINT32_T uRight, 
				   UINT32_T uBottom, 
				   UINT8_T uRed, 
				   UINT8_T uGreen, 
				   UINT8_T uBlue
				   )
{
	BOOL	fRes = FALSE;

	sOutDat.Type = VDISP_OUT_TYPE__FILL_RECT;
	sOutDat.u.FillRect.Color = VDISP_RGB(uRed, uGreen, uBlue);
	
	SetRectangle(&sOutDat.u.FillRect.Rectangle, uLeft, uTop, uRight, uBottom);

	//DBG_PRINTF("%s: sizeof(VDISP_OUT_DATA) = %d bytes\r\n", __FUNCTION__, sizeof(VDISP_OUT_DATA));

	fRes = WriteToUsbSerial((UINT8_PTR_T) &sOutDat, sizeof(VDISP_OUT_DATA));
	//if (fRes) {
	//	sOutDat.Type = VDISP_OUT_TYPE__NONE;
	//	fRes = WriteToUsbSerial((UINT8_PTR_T) &sOutDat, 2);
	//}

	return fRes;
}

BOOL UpdateDisplay()
{
	sOutDat.Type = VDISP_OUT_TYPE__UPDATE;
	return WriteToUsbSerial((UINT8_PTR_T) &sOutDat, sizeof(VDISP_OUT_DATA));
}

BOOL DrawLine(PVDISP_POINT pFromPoint, PVDISP_POINT pToPoint, UINT8_T uRed, UINT8_T uGreen, UINT8_T uBlue)
{
	UINT32_T	uiDeltaX = 0;
	UINT32_T	uiDeltaY = 0;
	UINT32_T	uiMaxDelta;
	UINT32_T	ui;
	UINT8_T		uQuad = 0;
	BOOL		fRes = FALSE;
	UINT32_T	X, Y;

	if (pFromPoint->X > pToPoint->X) {
		uiDeltaX = pFromPoint->X - pToPoint->X;
		uQuad |= VDISP__NEGATIVE_DELTA_X;
	} else {
		uiDeltaX = pToPoint->X - pFromPoint->X;
		uQuad |= VDISP__POSITIVE_DELTA_X;
	}

	if (pFromPoint->Y > pToPoint->Y) {
		uiDeltaY = pFromPoint->Y - pToPoint->Y;
		uQuad |= VDISP__NEGATIVE_DELTA_Y;
	} else {
		uiDeltaY = pToPoint->Y - pFromPoint->Y;
		uQuad |= VDISP__POSITIVE_DELTA_Y;
	}

	if (uiDeltaX > uiDeltaY) {
		uiMaxDelta = uiDeltaX;
	} else {
		uiMaxDelta = uiDeltaY;
	}

	switch (uQuad)
	{
	case VDISP__QUADRAN_0:
		for (ui = 0; ui < uiMaxDelta; ui++) {
			if (uiDeltaX > uiDeltaY) {
				X = pFromPoint->X + ui;
				Y = pFromPoint->Y + ((ui * uiDeltaY) / uiDeltaX);
			} else {
				Y = pFromPoint->Y + ui;
				if (uiDeltaY > 0) {
					X = pFromPoint->X + ((ui * uiDeltaX) / uiDeltaY);
				} else {
					X = pFromPoint->X;
				}
			}

			fRes = SetPixel(X, Y, uRed, uGreen, uBlue);
		}
		break;


	case VDISP__QUADRAN_1:
		for (ui = 0; ui < uiMaxDelta; ui++) {
			if (uiDeltaX > uiDeltaY) {
				X = pFromPoint->X - ui;
				Y = pFromPoint->Y + ((ui * uiDeltaY) / uiDeltaX);
			} else {
				Y = pFromPoint->Y + ui;
				if (uiDeltaY > 0) {
					X = pFromPoint->X - ((ui * uiDeltaX) / uiDeltaY);
				} else {
					X = pFromPoint->X;
				}
			}

			fRes = SetPixel(X, Y, uRed, uGreen, uBlue);
		}
		break;


	case VDISP__QUADRAN_2:
		for (ui = 0; ui < uiMaxDelta; ui++) {
			if (uiDeltaX > uiDeltaY) {
				X = pFromPoint->X - ui;
				Y = pFromPoint->Y - ((ui * uiDeltaY) / uiDeltaX);
			} else {
				Y = pFromPoint->Y - ui;
				if (uiDeltaY > 0) {
					X = pFromPoint->X - ((ui * uiDeltaX) / uiDeltaY);
				} else {
					X = pFromPoint->X;
				}
			}

			fRes = SetPixel(X, Y, uRed, uGreen, uBlue);
		}
		break;


	case VDISP__QUADRAN_3:
		for (ui = 0; ui < uiMaxDelta; ui++) {
			if (uiDeltaX > uiDeltaY) {
				X = pFromPoint->X + ui;
				Y = pFromPoint->Y - ((ui * uiDeltaY) / uiDeltaX);
			} else {
				Y = pFromPoint->Y - ui;
				if (uiDeltaY > 0) {
					X = pFromPoint->X + ((ui * uiDeltaX) / uiDeltaY);
				} else {
					X = pFromPoint->X;
				}
			}

			fRes = SetPixel(X, Y, uRed, uGreen, uBlue);
		}
		break;


	default:
		DBG_PRINTF("%s: Unexpected quadran value: 0x%X\r\n", __FUNCTION__, uQuad);
		break;
	}

	return fRes;
}

BOOL DrawHorizontalLine(UINT32_T uLeft, UINT32_T uTop, UINT32_T uLength, UINT8_T uRed, UINT8_T uGreen, UINT8_T uBlue)
{
	UINT32_T	u = 0;
	BOOL		fRes = FALSE;
	UINT32_T	X;

	for (u = 0; u < uLength; u++) {
		X = uLeft + u;
		fRes = SetPixel(X, uTop, uRed, uGreen, uBlue);
	}

	return fRes;
}

BOOL DrawVerticalLine(UINT32_T uLeft, UINT32_T uTop, UINT32_T uLength, UINT8_T uRed, UINT8_T uGreen, UINT8_T uBlue)
{
	UINT32_T	u = 0;
	BOOL		fRes = FALSE;
	UINT32_T	Y;

	for (u = 0; u < uLength; u++) {
		Y = uTop + u;
		fRes = SetPixel(uLeft, Y, uRed, uGreen, uBlue);
	}

	return fRes;
}

static UINT32_T Root2(UINT32_T uValue)
{
	UINT32_T	u;

	if (uValue == 0) return 0;

	for (u = 0; u < 0xFFFFFFFF; u++) {
		if ((u * u) >= uValue) break;
	}

	return u;
}

BOOL DrawCircle(UINT32_T uX, UINT32_T uY, UINT32_T uRadius, UINT8_T uRed, UINT8_T uGreen, UINT8_T uBlue)
{
	UINT32_T	v, w;
	UINT32_T	m;
	BOOL		fRes = FALSE;

	for (v = 0; v <= uRadius; v++) {
		m = (uRadius * uRadius) - (v * v);
		w = Root2(m);

		/* Clock wise */
		fRes = SetPixel((uX + v), (uY - w), uRed, uGreen, uBlue);
		fRes = SetPixel((uX + w), (uY + v), uRed, uGreen, uBlue);
		fRes = SetPixel((uX - v), (uY + w), uRed, uGreen, uBlue);
		fRes = SetPixel((uX - w), (uY - v), uRed, uGreen, uBlue);

		/* Counter clock wise */
		fRes = SetPixel((uX - v), (uY - w), uRed, uGreen, uBlue);
		fRes = SetPixel((uX - w), (uY + v), uRed, uGreen, uBlue);
		fRes = SetPixel((uX + v), (uY + w), uRed, uGreen, uBlue);
		fRes = SetPixel((uX + w), (uY - v), uRed, uGreen, uBlue);
	}

	return fRes;
}

BOOL DrawRect(UINT32_T uLeft, UINT32_T uTop, UINT32_T uRight, UINT32_T uBottom, UINT8_T uRed, UINT8_T uGreen, UINT8_T uBlue)
{
	BOOL	fRes = FALSE;

	fRes = DrawHorizontalLine(uLeft, uTop, (uRight - uLeft), uRed, uGreen, uBlue);

	if (fRes)
		fRes = DrawVerticalLine((uRight - 1), uTop, (uBottom - uTop), uRed, uGreen, uBlue);

	if (fRes)
		fRes = DrawVerticalLine(uLeft, uTop, (uBottom - uTop), uRed, uGreen, uBlue);

	if (fRes)
		fRes = DrawHorizontalLine(uLeft, (uBottom - 1), (uRight - uLeft), uRed, uGreen, uBlue);

	return fRes;
}

BOOL GetInput(PVDISP_IN_DATA pInput)
{
	BOOL	fRes = FALSE;

	fRes = ReadFromUsbSerial((UINT8_PTR_T) pInput, sizeof(VDISP_IN_DATA), 0);

	return fRes;
}