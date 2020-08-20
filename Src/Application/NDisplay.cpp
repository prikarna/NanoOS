/*
 * File    : NDisplay.cpp
 * Remark  : Display and related class implementation.
 *
 */

#include "NDisplay.h"

#define NDISP__POSITIVE_DELTA_X		0x01
#define NDISP__NEGATIVE_DELTA_X		0x02
#define NDISP__POSITIVE_DELTA_Y		0x04
#define NDISP__NEGATIVE_DELTA_Y		0x08

#define NDISP__QUADRAN_0			(NDISP__POSITIVE_DELTA_X | NDISP__POSITIVE_DELTA_Y)
#define NDISP__QUADRAN_1			(NDISP__NEGATIVE_DELTA_X | NDISP__POSITIVE_DELTA_Y)
#define NDISP__QUADRAN_2			(NDISP__NEGATIVE_DELTA_X | NDISP__NEGATIVE_DELTA_Y)
#define NDISP__QUADRAN_3			(NDISP__POSITIVE_DELTA_X | NDISP__NEGATIVE_DELTA_Y)

namespace NWindow
{
/* Driver class implementation begin */
Driver::Driver()
{
	m_bEnabled = false;
}

Driver::~Driver()
{
}

bool Driver::Initialize()
{
	if (IsUsbSerialReady() == FALSE)
		return false;

	if (m_bEnabled) return true;

	m_bEnabled = true;
	return true;
}

void Driver::UnInitialize()
{
	m_bEnabled = false;
}

bool Driver::SetPixel(int iX, int iY, COLORREF col, COLORREF *pRetCol)
{
	if (!m_bEnabled) {
		return false;
	}

	BOOL	fRes = FALSE;

	m_OutDat.Type = VDISP_OUT_TYPE__SET_PIXEL;
	m_OutDat.u.Pixel.Color = col;
	m_OutDat.u.Pixel.X = iX;
	m_OutDat.u.Pixel.Y = iY;

	fRes = WriteToUsbSerial((UINT8_PTR_T) &m_OutDat, sizeof(VDISP_OUT_DATA));
	return (fRes) ? true : false;
}

bool Driver::GetPixel(int iX, int iY, COLORREF &retCol)
{
	if (!m_bEnabled) return false;

	BOOL	fRes = FALSE;

	m_OutDat.Type = VDISP_OUT_TYPE__GET_PIXEL;
	m_OutDat.u.Pixel.X = iX;
	m_OutDat.u.Pixel.Y = iY;

	fRes = WriteToUsbSerial((UINT8_PTR_T) &m_OutDat, sizeof(VDISP_OUT_DATA));
	if (fRes) {
		VDISP_IN_DATA	InDat;
		MemSet((UINT8_PTR_T) &InDat, 0, sizeof(VDISP_IN_DATA));
		fRes = ReadFromUsbSerial((UINT8_PTR_T) &InDat, sizeof(VDISP_IN_DATA), 0);
		if (fRes) {
			if (InDat.Type == VDISP_IN_TYPE__PIXEL) {
				retCol = InDat.Color;
			} else {
				fRes = FALSE;
			}
		}
	}
	
	return (fRes) ? true : false;
}

bool Driver::Fill(COLORREF col)
{
	if (!m_bEnabled) return false;

	BOOL	fRes = FALSE;

	m_OutDat.Type = VDISP_OUT_TYPE__FILL_RECT;
	m_OutDat.u.FillRect.Color = col;
	m_OutDat.u.FillRect.Rectangle.Left = 0;
	m_OutDat.u.FillRect.Rectangle.Top = 0;
	m_OutDat.u.FillRect.Rectangle.Right = VDISP_WIDTH;
	m_OutDat.u.FillRect.Rectangle.Bottom = VDISP_HEIGHT;

	fRes = WriteToUsbSerial((UINT8_PTR_T) &m_OutDat, sizeof(VDISP_OUT_DATA));

	return fRes;
}

bool Driver::Fill(COLORREF col, VDISP_RECTANGLE &Rect)
{
	if (!m_bEnabled) return false;

	BOOL	fRes = FALSE;

	m_OutDat.Type = VDISP_OUT_TYPE__FILL_RECT;
	m_OutDat.u.FillRect.Color = col;
	m_OutDat.u.FillRect.Rectangle = Rect;

	fRes = WriteToUsbSerial((UINT8_PTR_T) &m_OutDat, sizeof(VDISP_OUT_DATA));

	return fRes;
}

bool Driver::GetRect(VDISP_RECTANGLE &Rect)
{
	if (!m_bEnabled) return false;
	
	Rect.Left = 0;
	Rect.Top = 0;
	Rect.Right = VDISP_WIDTH;
	Rect.Bottom = VDISP_HEIGHT;

	return true;
}
/* Driver class implementaion end */

Font::Font():
m_pFont(NULL)
{
}

Font::~Font()
{
}

const unsigned char Font::m_Data[1156] =
{
	0x08,	// Width
	0x0C,	// Height
	0x5F,	// Numb. of chars. = 95
	0x20,	// Starting char. = 0x20 (Space char. in ASCII)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//  [Space]
	0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x10,0x10,0x00,	// !
	0x00,0x28,0x28,0x28,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// "
	0x00,0x14,0x14,0x14,0x7E,0x28,0xFC,0x50,0x50,0x50,0x00,0x00,	// #
	0x10,0x38,0x54,0x50,0x30,0x18,0x14,0x14,0x54,0x38,0x10,0x00,	// $
	0x00,0x42,0xA4,0xA8,0x48,0x10,0x24,0x2A,0x4A,0x84,0x00,0x00,	// %
	0x00,0x20,0x50,0x50,0x20,0x50,0x8A,0x8C,0x54,0x22,0x00,0x00,	// &
	0x00,0x10,0x10,0x10,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// '
	0x00,0x08,0x10,0x20,0x20,0x20,0x20,0x20,0x10,0x08,0x00,0x00,	// (
	0x00,0x10,0x08,0x04,0x04,0x04,0x04,0x04,0x08,0x10,0x00,0x00,	// )
	0x00,0x54,0x38,0x10,0x38,0x54,0x00,0x00,0x00,0x00,0x00,0x00,	// *
	0x00,0x00,0x00,0x10,0x10,0x10,0xFE,0x10,0x10,0x10,0x00,0x00,	// +
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x08,0x10,0x20,	// ,
	0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,	// -
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,	// .
	0x00,0x04,0x04,0x08,0x08,0x10,0x10,0x20,0x20,0x40,0x40,0x00,	// /
	0x00,0x18,0x24,0x42,0x42,0x42,0x42,0x42,0x24,0x18,0x00,0x00,	// 0
	0x00,0x10,0x30,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,	// 1
	0x00,0x38,0x44,0x84,0x04,0x08,0x10,0x20,0x40,0xFC,0x00,0x00,	// 2
	0x00,0x78,0x84,0x04,0x04,0x38,0x04,0x04,0x84,0x78,0x00,0x00,	// 3
	0x00,0x08,0x18,0x18,0x28,0x28,0x48,0xFC,0x08,0x08,0x00,0x00,	// 4
	0x00,0xFC,0x80,0x80,0x80,0xF8,0x04,0x04,0x04,0xF8,0x00,0x00,	// 5
	0x00,0x78,0x84,0x80,0x80,0xF8,0x84,0x84,0x84,0x78,0x00,0x00,	// 6
	0x00,0xFC,0x08,0x10,0x10,0x20,0x20,0x40,0x40,0x40,0x00,0x00,	// 7
	0x00,0x78,0x84,0x84,0x84,0x78,0x84,0x84,0x84,0x78,0x00,0x00,	// 8
	0x00,0x78,0x84,0x84,0x84,0x7C,0x04,0x04,0x84,0x78,0x00,0x00,	// 9
	0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x30,0x30,0x00,0x00,0x00,	// :
	0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x30,0x30,0x10,0x10,0x20,	// ;
	0x00,0x00,0x04,0x18,0x60,0x80,0x60,0x18,0x04,0x00,0x00,0x00,	// <
	0x00,0x00,0x00,0x00,0x7C,0x00,0x7C,0x00,0x00,0x00,0x00,0x00,	// =
	0x00,0x00,0x80,0x60,0x18,0x04,0x18,0x60,0x80,0x00,0x00,0x00,	// >
	0x00,0x38,0x44,0x04,0x04,0x08,0x10,0x10,0x00,0x10,0x00,0x00,	// ?
	0x00,0x38,0x44,0x82,0x9A,0xAA,0xAA,0x96,0x40,0x3C,0x00,0x00,	// @
	0x00,0x78,0x84,0x84,0x84,0xFC,0x84,0x84,0x84,0x84,0x00,0x00,	// A
	0x00,0xF8,0x84,0x84,0x84,0xF8,0x84,0x84,0x84,0xF8,0x00,0x00,	// B
	0x00,0x78,0x84,0x80,0x80,0x80,0x80,0x80,0x84,0x78,0x00,0x00,	// C
	0x00,0xF0,0x88,0x84,0x84,0x84,0x84,0x84,0x88,0xF0,0x00,0x00,	// D
	0x00,0xFC,0x80,0x80,0x80,0xF0,0x80,0x80,0x80,0xFC,0x00,0x00,	// E
	0x00,0xFC,0x80,0x80,0x80,0xF0,0x80,0x80,0x80,0x80,0x00,0x00,	// F
	0x00,0x78,0x84,0x80,0x80,0xBC,0x84,0x84,0x84,0x78,0x00,0x00,	// G
	0x00,0x84,0x84,0x84,0x84,0xFC,0x84,0x84,0x84,0x84,0x00,0x00,	// H
	0x00,0x38,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00,	// I
	0x00,0x1C,0x08,0x08,0x08,0x08,0x08,0x08,0x88,0x70,0x00,0x00,	// J
	0x00,0x84,0x88,0x90,0xA0,0xC0,0xA0,0x90,0x88,0x84,0x00,0x00,	// K
	0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7C,0x00,0x00,	// L
	0x00,0x84,0x84,0xCC,0xCC,0xB4,0x84,0x84,0x84,0x84,0x00,0x00,	// M
	0x00,0x84,0xC4,0xC4,0xA4,0xB4,0x94,0x8C,0x8C,0x84,0x00,0x00,	// N
	0x00,0x78,0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x78,0x00,0x00,	// O
	0x00,0xF8,0x84,0x84,0x84,0xF8,0x80,0x80,0x80,0x80,0x00,0x00,	// P
	0x00,0x78,0x84,0x84,0x84,0x84,0xA4,0x94,0x88,0x74,0x00,0x00,	// Q
	0x00,0xF8,0x84,0x84,0x84,0xF8,0x88,0x84,0x84,0x84,0x00,0x00,	// R
	0x00,0x78,0x84,0x80,0x80,0x78,0x04,0x04,0x84,0x78,0x00,0x00,	// S
	0x00,0xFE,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,	// T
	0x00,0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x84,0x78,0x00,0x00,	// U
	0x00,0x82,0x82,0x82,0x44,0x44,0x28,0x28,0x10,0x10,0x00,0x00,	// V
	0x00,0x84,0x84,0x84,0x84,0xB4,0xCC,0xCC,0x84,0x84,0x00,0x00,	// W
	0x00,0x82,0x82,0x44,0x28,0x10,0x28,0x44,0x82,0x82,0x00,0x00,	// X
	0x00,0x82,0x82,0x44,0x28,0x10,0x10,0x10,0x10,0x10,0x00,0x00,	// Y
	0x00,0xFC,0x08,0x10,0x10,0x20,0x20,0x20,0x40,0xFC,0x00,0x00,	// Z
	0x00,0x38,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x38,0x00,0x00,	// [
	0x00,0x40,0x40,0x20,0x20,0x10,0x10,0x08,0x08,0x04,0x04,0x00,	// /* \ */ [Backslash]
	0x00,0x38,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x38,0x00,0x00,	// ]
	0x00,0x10,0x28,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// ^
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,	// _
	0x00,0x40,0x20,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// `
	0x00,0x00,0x00,0x00,0x78,0x04,0x7C,0x84,0x84,0x7C,0x00,0x00,	// a
	0x00,0x80,0x80,0x80,0xF8,0x84,0x84,0x84,0x84,0xF8,0x00,0x00,	// b
	0x00,0x00,0x00,0x00,0x78,0x84,0x80,0x80,0x84,0x78,0x00,0x00,	// c
	0x00,0x04,0x04,0x04,0x7C,0x84,0x84,0x84,0x84,0x7C,0x00,0x00,	// d
	0x00,0x00,0x00,0x00,0x78,0x84,0xFC,0x80,0x84,0x78,0x00,0x00,	// e
	0x00,0x1C,0x20,0x20,0x78,0x20,0x20,0x20,0x20,0x20,0x00,0x00,	// f
	0x00,0x00,0x00,0x00,0x7C,0x84,0x84,0x84,0x84,0x7C,0x04,0xF8,	// g
	0x00,0x80,0x80,0x80,0xF8,0x84,0x84,0x84,0x84,0x84,0x00,0x00,	// h
	0x00,0x00,0x10,0x00,0x70,0x10,0x10,0x10,0x10,0x7C,0x00,0x00,	// i
	0x00,0x00,0x04,0x00,0x1C,0x04,0x04,0x04,0x04,0x04,0x44,0x38,	// j
	0x00,0x40,0x40,0x40,0x44,0x48,0x60,0x50,0x48,0x44,0x00,0x00,	// k
	0x00,0x30,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x7C,0x00,0x00,	// l
	0x00,0x00,0x00,0x00,0xEC,0x92,0x92,0x92,0x92,0x92,0x00,0x00,	// m
	0x00,0x00,0x00,0x00,0xF8,0x84,0x84,0x84,0x84,0x84,0x00,0x00,	// n
	0x00,0x00,0x00,0x00,0x78,0x84,0x84,0x84,0x84,0x78,0x00,0x00,	// o
	0x00,0x00,0x00,0x00,0xF8,0x84,0x84,0x84,0x84,0xF8,0x80,0x80,	// p
	0x00,0x00,0x00,0x00,0x7C,0x84,0x84,0x84,0x84,0x7C,0x04,0x04,	// q
	0x00,0x00,0x00,0x00,0xB8,0xC4,0x80,0x80,0x80,0x80,0x00,0x00,	// r
	0x00,0x00,0x00,0x00,0x78,0x84,0x60,0x18,0x84,0x78,0x00,0x00,	// s
	0x00,0x20,0x20,0x7C,0x20,0x20,0x20,0x20,0x20,0x1C,0x00,0x00,	// t
	0x00,0x00,0x00,0x00,0x84,0x84,0x84,0x84,0x84,0x7C,0x00,0x00,	// u
	0x00,0x00,0x00,0x00,0x82,0x82,0x82,0x44,0x28,0x10,0x00,0x00,	// v
	0x00,0x00,0x00,0x00,0x82,0x82,0x92,0x92,0x92,0x6C,0x00,0x00,	// w
	0x00,0x00,0x00,0x00,0x84,0x48,0x30,0x30,0x48,0x84,0x00,0x00,	// x
	0x00,0x00,0x00,0x00,0x44,0x44,0x44,0x28,0x10,0x10,0x20,0xC0,	// y
	0x00,0x00,0x00,0x00,0xFC,0x08,0x10,0x20,0x40,0xFC,0x00,0x00,	// z
	0x00,0x0C,0x10,0x10,0x10,0x70,0x10,0x10,0x10,0x0C,0x00,0x00,	// {
	0x00,0x10,0x10,0x10,0x10,0x00,0x10,0x10,0x10,0x10,0x00,0x00,	// |
	0x00,0x60,0x10,0x10,0x10,0x1C,0x10,0x10,0x10,0x60,0x00,0x00,	// }
	0x00,0x20,0x54,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// ~
	0x00,0xFC,0x84,0x84,0x84,0x84,0x84,0x84,0x84,0xFC,0x00,0x00		// [Unknown]

};

void Font::Destroy()
{
	m_pFont = NULL;
}

bool Font::Create(PFONT pFont)
{
	if (!pFont) {
		m_pFont = reinterpret_cast<PFONT>(const_cast<unsigned char *>(&m_Data[0]));
	} else {
		m_pFont = pFont;
	}

	return true;
}

int Font::CharWidth()
{
	if (!m_pFont) return -1;
	return static_cast<int>(m_pFont->Width);
}

int Font::CharHeight()
{
	if (!m_pFont) return -1;
	return static_cast<int>(m_pFont->Height);
}

int Font::NumberOfChars()
{
	if (!m_pFont) return -1;
	return static_cast<int>(m_pFont->NumbOfChars);
}

bool Font::GetCharData(char ch, unsigned char **ppData, int *piDataLength)
{
	if	((ppData == 0) ||
		(m_pFont == 0))
	{
		return false;
	}

	unsigned char *p = &(m_pFont->Data[0]);

	if ((ch < ' ') ||
		(ch > '~'))
	{
		p += (static_cast<int>(m_pFont->NumbOfChars) * m_pFont->Height);	// Point to unknown char. symbol
	}
	else
	{
		int iOffset = (static_cast<int>(ch - m_pFont->StartingChar) * m_pFont->Height);
		p += iOffset;
	}

	*ppData = p;
	if (piDataLength) *piDataLength = m_pFont->Height;

	return true;
}

bool Font::GetFirstChar(unsigned char **ppData)
{
	if ((!m_pFont) ||
		(!ppData))
	{
		return false;
	}

	*ppData = &(m_pFont->Data[0]);
	return true;
}

Rectangle::Rectangle()
{
	Reset();
}

Rectangle Rectangle::operator =(Rectangle RightRect)
{
	Left = RightRect.Left;
	Top = RightRect.Top;
	Right = RightRect.Right;
	Bottom = RightRect.Bottom;
	return *this;
}

bool Rectangle::operator ==(Rectangle RightRect)
{
	if ((Left == RightRect.Left) &&
		(Top == RightRect.Top) &&
		(Right == RightRect.Right) &&
		(Bottom == RightRect.Bottom))
	{
		return true;
	}
	return false;
}

bool Rectangle::operator !=(Rectangle RightRect)
{
	if ((Left != RightRect.Left) ||
		(Top != RightRect.Top) ||
		(Right != RightRect.Right) ||
		(Bottom != RightRect.Bottom))
	{
		return true;
	}
	return false;
}

Rectangle Rectangle::operator +=(int Value)
{
	Left -= Value;
	Top -= Value;
	Right += Value;
	Bottom += Value;
	return *this;
}

Rectangle Rectangle::operator &=(Rectangle RightRect)
{
	Left = (Left > RightRect.Left) ? Left : RightRect.Left;
	Right = (Right < RightRect.Right) ? Right : RightRect.Right;
	Top = (Top > RightRect.Top) ? Top : RightRect.Top;
	Bottom = (Bottom < RightRect.Bottom) ? Bottom : RightRect.Bottom;
	return *this;
}

Rectangle Rectangle::operator |=(Rectangle RightRect)
{
	Left = (Left < RightRect.Left) ? Left : RightRect.Left;
	Right = (Right > RightRect.Right) ? Right : RightRect.Right;
	Top = (Top < RightRect.Top) ? Top : RightRect.Top;
	Bottom = (Bottom > RightRect.Bottom) ? Bottom : RightRect.Bottom;
	return *this;
}

Rectangle Rectangle::operator =(VDISP_RECTANGLE RightRect)
{
	Left = RightRect.Left;
	Top = RightRect.Top;
	Right = RightRect.Right;
	Bottom = RightRect.Bottom;
	return *this;
}

Rectangle::operator VDISP_RECTANGLE()
{
	VDISP_RECTANGLE rc;
	rc.Left = Left;
	rc.Top = Top;
	rc.Right = Right;
	rc.Bottom = Bottom;
	return rc;
}

bool Rectangle::IsEmpty()
{
	if ((Left == 0) &&
		(Top == 0) &&
		(Right == 0) &&
		(Bottom == 0))
	{
		return true;
	}
	return false;
}

void Rectangle::Reset()
{
	Left = 0;
	Top = 0;
	Right = 0;
	Bottom = 0;
}

void Rectangle::Set(int iLeft, int iTop, int iRight, int iBottom)
{
	Left = iLeft;
	Top = iTop;
	Right = iRight;
	Bottom = iBottom;
}

void Rectangle::Offset(int idx, int idy)
{
	Left += idx;
	Top += idy;
	Right += idx;
	Bottom += idy;
}

int Rectangle::Width()
{
	return (Right - Left);
}

int Rectangle::Height()
{
	return (Bottom - Top);
}

Display::Display(void)
{
	m_chNL			= '\n';
	m_chNull		= '\0';
	m_chWordDelim	= ' ';

	m_Font.Create(0);
	m_clText = RGB(0, 0, 0);			// Black
	m_clTextBkg = RGB(255, 255, 255);	// White

	m_bTextCalc		= false;

	m_iTextScale	= 1;
	m_bTempTxtCol	= false;
	m_iLineSpace	= 2;
	m_eTextAlign	= TextAlign::LeftTop;

	m_bEndEllipsis	= false;
	m_bWordWrap		= true;
	m_bMultiLine	= false;

	__p				= 0;
	__pLine			= 0;
}

Display::~Display(void)
{
}

// Yes, this is bruthe force
unsigned int Display::_Root2(unsigned int uValue)
{
	unsigned int	u;

	if (uValue == 0) return 0;

	for (u = 0; u < 0xFFFFFFFF; u++) {
		if ((u * u) >= uValue) break;
	}

	return u;
}

void Display::_RestoreTextColor()
{
	m_clTextBkg	= m_clPrevTextBkg;
	m_clText	= m_clPrevText;
	m_bTempTxtCol = false;
}

bool Display::_GetWordWidth(const char *szWord, bool bIncDelim, int iMaxWidth, int &iWordLength, int &iWordWidth)
{
	if (!szWord) return false;

	char * p = const_cast<char *>(szWord);
	int iW = m_Font.m_pFont->Width * m_iTextScale;
	
	iWordWidth = 0;
	iWordLength = 0;
	while (*p != m_chWordDelim) {
		if (*p == m_chNull) break;
		if (*p == m_chNL) {
			p++;
			break;
		}
		if ((iWordWidth + iW) > iMaxWidth) break;
		iWordWidth += iW;
		p++;
		iWordLength++;
	}

	if ((bIncDelim) && (*p == m_chWordDelim)) {
		iWordWidth += iW;
		iWordLength++;
	}

	return (iWordWidth > 0) ? true : false;
}

bool Display::_GetLineText(char *pStart, int iMaxWidth, char **ppNext, int & iStrLen, int & iStrWidth)
{
	if ((iMaxWidth <= 0) || (pStart == 0)) 
		return false;
	
	int iChW = (m_Font.CharWidth() * m_iTextScale);
	
	iStrLen = 0;
	iStrWidth = 0;

	if (m_bMultiLine & m_bWordWrap)
	{
		int iWordLen = 0;
		int iWordWidth = 0;
		bool b = false;

		while (*pStart != m_chNull)
		{
			b = _GetWordWidth(pStart, true, iMaxWidth, iWordLen, iWordWidth);
			if (!b) break;
			
			if (iWordWidth > iMaxWidth) {
				iStrWidth = iWordWidth;
				iStrLen = iWordLen;
				pStart += iWordLen;
				break;
			}

			if ((iStrWidth + iWordWidth) > iMaxWidth) break;

			iStrLen += iWordLen;
			iStrWidth += iWordWidth;
			pStart += iWordLen;
		}
	}
	else
	{
		while (*pStart != m_chNull)
		{
			if (*pStart == m_chNL) {
				pStart++;
				break;
			}
			if ((iStrWidth + iChW) > iMaxWidth) break;
			iStrLen++;
			iStrWidth += iChW;
			
			pStart++;
		}
	}

	if (ppNext) *ppNext = pStart;

	return (iStrWidth > 0) ? true : false;
}

bool Display::_DrawChar(char ch, int iX, int iY)
{
	unsigned char *pCharData;
	unsigned char uc;
	bool b;
	
	b = m_Font.GetCharData(ch, &pCharData, 0);
	if (!b) return false;
	
	int	iMaxY = iY + m_Font.m_pFont->Height;
	int iMaxX = iX + m_Font.m_pFont->Width;

	for (int y = iY; y < iMaxY; y++)
	{
		uc = *pCharData++;

		for (int x = iX; x < iMaxX; x++)
		{
			if (uc & (0x80 >> (x - iX))) {
				b = m_Drv.SetPixel(x, y, m_clText, 0);
			} else {
				b = m_Drv.SetPixel(x, y, m_clTextBkg, 0);
			}
			if (!b) break;
		}
		if (!b) break;
	}

	return b;
}

bool Display::_DrawChar(char ch, int iX, int iY, int & iNextX)
{
	unsigned char *pCharData;
	unsigned char uc;
	bool b = false;

	if (m_bTextCalc) {
		iNextX = iX + (m_Font.m_pFont->Width * m_iTextScale);
		return true;
	}
	
	b = m_Font.GetCharData(ch, &pCharData, 0);
	if (!b) return false;
	
	int	iMaxY = iY + m_Font.m_pFont->Height;
	int iMaxX = iX + m_Font.m_pFont->Width;
	int	iXPos = iX;
	int	iYPos = iY;

	for (int y = iY; y < iMaxY; y++)
	{
		uc = *pCharData++;

		for (int sy = 0; sy < m_iTextScale; sy++)
		{
			iXPos = iX;
			for (int x = iX; x < iMaxX; x++)
			{
				for (int sx = 0; sx < m_iTextScale; sx++)
				{
					if (uc & (0x80 >> (x - iX))) {
						b = m_Drv.SetPixel(iXPos, iYPos, m_clText, 0);
					} else {
						b = m_Drv.SetPixel(iXPos, iYPos, m_clTextBkg, 0);
					}
					if (!b) break;
					iXPos++;
				}
			}
			if (!b) break;
			iYPos++;
		}
	}

	if (b) iNextX = iXPos;

	return b;
}

bool Display::_DrawEllipsis(int iX, int iY, int &iNextX)
{
	bool b = false;
	int iXPos = iX;

	for (int i = 0; i < 3; i++)
	{
		b = _DrawChar('.', iXPos, iY, iXPos);
		if (!b) break;
	}

	if (b) iNextX = iXPos;

	return b;
}

bool Display::_DrawCaret(int iX, int iY, bool bVisible)
{
	bool b = false;
	int	iMaxY = iY + (m_Font.m_pFont->Height * m_iTextScale);

	for (int y = iY; y < iMaxY; y++)
	{
		if (bVisible) {
			b = m_Drv.SetPixel(iX, y, m_clText, 0);
		} else {
			b = m_Drv.SetPixel(iX, y, m_clTextBkg, 0);
		}
	}

	return b;
}

bool Display::_CalcStartPos(Rectangle & rcLine, int iStringWidth, Point & ptRes, int & iXMaxRes)
{
	ptRes.x = 0;
	ptRes.y = 0;
	iXMaxRes = 0;

	int dx = 0, dy = 0;
	bool b = true;

	switch (m_eTextAlign)
	{
	case TextAlign::LeftTop:
		ptRes.x = rcLine.Left;
		ptRes.y = rcLine.Top;
		iXMaxRes = rcLine.Left + iStringWidth;
		break;

	case TextAlign::TopRight:
		ptRes.x = rcLine.Right - iStringWidth;
		ptRes.y = rcLine.Top;
		iXMaxRes = rcLine.Right;
		break;

	case TextAlign::TopHCenter:
		dx = rcLine.Width() - iStringWidth;
		if (dx <= 0) {
			ptRes.x = rcLine.Left;
		} else {
			ptRes.x = rcLine.Left + (dx / 2);
		}
		ptRes.y = rcLine.Top;
		iXMaxRes = ptRes.x + iStringWidth;
		break;

	case TextAlign::BottomHCenter:
		if (m_bMultiLine) {
			dx = rcLine.Width() - iStringWidth;
			if (dx <= 0) {
				ptRes.x = rcLine.Left;
			} else {
				ptRes.x = rcLine.Left + (dx / 2);
			}
			ptRes.y = rcLine.Top;
			iXMaxRes = ptRes.x + iStringWidth;
		} else {
			dx = rcLine.Width() - iStringWidth;
			if (dx <= 0) {
				ptRes.x = rcLine.Left;
			} else {
				ptRes.x = rcLine.Left + (dx / 2);
			}
			ptRes.y = rcLine.Bottom - ((m_Font.m_pFont->Height * m_iTextScale) + m_iLineSpace);
			iXMaxRes = ptRes.x + iStringWidth;
		}
		break;

	case TextAlign::Center:
		if (m_bMultiLine) {
			dx = rcLine.Width() - iStringWidth;
			if (dx <= 0) {
				ptRes.x = rcLine.Left;
			} else {
				ptRes.x = rcLine.Left + (dx / 2);
			}
			ptRes.y = rcLine.Top;
			iXMaxRes = ptRes.x + iStringWidth;
		} else {
			dx = rcLine.Width() - iStringWidth;
			if (dx <= 0) {
				ptRes.x = rcLine.Left;
			} else {
				ptRes.x = rcLine.Left + (dx / 2);
			}
			dy = rcLine.Height() - ((m_Font.m_pFont->Height * m_iTextScale) + m_iLineSpace);
			if (dy <= 0) {
				ptRes.y = rcLine.Top;
			} else {
				ptRes.y = rcLine.Top + (dy / 2);
			}
			iXMaxRes = ptRes.x + iStringWidth;
		}
		break;

	case TextAlign::LeftVCenter:
		ptRes.x = rcLine.Left;
		dy = rcLine.Height() - ((m_Font.m_pFont->Width * m_iTextScale) + m_iLineSpace);
		if (dy <= 0) {
			ptRes.y = rcLine.Top;
		} else {
			ptRes.y = rcLine.Top + (dy / 2);
		}
		iXMaxRes = rcLine.Left + iStringWidth;
		break;

	case TextAlign::RightVCenter:
		ptRes.x = rcLine.Right - iStringWidth;
		dy = rcLine.Height() - ((m_Font.m_pFont->Width * m_iTextScale) + m_iLineSpace);
		if (dy <= 0) {
			ptRes.y = rcLine.Top;
		} else {
			ptRes.y = rcLine.Top + (dy / 2);
		}
		iXMaxRes = rcLine.Right;
		break;

	default:
		b = false;
		break;
	}

	return b;
}

bool Display::_WriteLineText(
							 char *pLine, 
							 char *pStartChar, 
							 int iStringLength, 
							 int iLineRight, 
							 Point & ptWrite, 
							 char **ppNext
							 )
{
	if (!pLine) return false;

	int iX = ptWrite.x;
	bool b = false;
	int iW = m_Font.m_pFont->Width * m_iTextScale;

	for (int iLen = 0; iLen < iStringLength; iLen++)
	{
		if (*pLine == m_chNull) break;
		if (m_bTextCalc) {
			if (pLine == pStartChar) {
				m_bTextCalc = false;
			}
		}
		if (m_bMultiLine) {
			b = _DrawChar(*pLine, iX, ptWrite.y, iX);
		} else {
			if (m_bEndEllipsis) {
				if (iX < (iLineRight - (iW * 3))) {
					b = _DrawChar(*pLine, iX, ptWrite.y, iX);
				} else {
					b = _DrawChar('.', iX, ptWrite.y, iX);
				}
			} else {
				b = _DrawChar(*pLine, iX, ptWrite.y, iX);
			}
		}
		if (!b) break;
		pLine++;
	}

	ptWrite.x = iX;

	// Padd
	//while ((iX + iW) <= iLineRight) {
	//	b = _DrawChar(m_chWordDelim, iX, ptWrite.y, iX);
	//}

	if (*pLine == m_chNL) pLine++;

	if (ppNext) *ppNext = pLine;
	return b;
}

bool Display::_NextLine(int &iCurX, int &iCurY, int iLeft, int iCharHeight)
{
	iCurX = iLeft;
	iCurY += iCharHeight;
	return true;
}

int Display::_Wrap(char *pCur, int iLeft, int iRight, int iCurX, int iCurY, int iCharWidth, char **ppNext)
{
	if (!pCur) return -1;

	if (*pCur == m_chWordDelim) {
		if (ppNext) *ppNext = pCur;
		return 0;
	}

	int iWrapX = iCurX;
	int iWrapLen = 0;
	bool b = false;
	char *p = pCur;

	// Count how many chars. (or total width in the current line) will be wrapped
	while (*p != m_chWordDelim)
	{
		if (iWrapX <= iLeft) break;
		iWrapX -= iCharWidth;
		p--;
		iWrapLen++;
	}

	// Delete (or write spaces) already writen chars. in the line if have word delim.
	if (*p == m_chWordDelim) {
		iWrapX += iCharWidth;
		while (iWrapX <= iRight) {
			b = _DrawChar(' ', iWrapX, iCurY, iWrapX);
			if (!b) break;
		}
		if (ppNext) *ppNext = ++p;
	} else {
		if (ppNext) *ppNext = pCur;
	}

	return iWrapLen;
}

bool Display::Create()
{
	return m_Drv.Initialize();
}

void Display::Destroy()
{
	m_Drv.UnInitialize();
}

bool Display::GetRect(Rectangle &rect)
{
	VDISP_RECTANGLE rc = rect;
	bool b = m_Drv.GetRect(rc);
	if (b) rect = rc;
	return b;
}

bool Display::SetPixel(int iX, int iY, COLORREF col, COLORREF *pRetCol)
{
	bool b = m_Drv.SetPixel(iX, iY, col, pRetCol);
	return b;
}

bool Display::GetPixel(int iX, int iY, COLORREF &clRes)
{
	bool b = m_Drv.GetPixel(iX, iY, clRes);
	return b;
}

bool Display::Fill(COLORREF col)
{
	return m_Drv.Fill(col);
}

bool Display::Fill(COLORREF col, Rectangle &rect)
{
	VDISP_RECTANGLE rc = rect;
	bool b = m_Drv.Fill(col, rc);
	if (b) rect = rc;
	return b;
}

bool Display::DrawRect(int iX, int iY, int iWidth, int iHeigth, COLORREF clLine)
{
	bool b = false;

	for (int x = iX; x < (iWidth + iX); x++)
	{
		b = m_Drv.SetPixel(x, iY, clLine, 0);
		if (!b) break;
		b = m_Drv.SetPixel(x, (iY + iHeigth), clLine, 0);
		if (!b) break;
	}
	if (!b) return false;

	for (int y = iY; y < (iHeigth + iY); y++)
	{
		b = m_Drv.SetPixel(iX, y, clLine, 0);
		if (!b) break;
		b = m_Drv.SetPixel((iX + iWidth), y, clLine, 0);
		if (!b) break;
	}
	if (!b) return false;

	return b;
}

bool Display::DrawRect(Rectangle & rect, COLORREF clLine)
{
	bool b = false;
	int x,y;

	for (x = rect.Left; x <= rect.Right; x++)
	{
		b = m_Drv.SetPixel(x, rect.Top, clLine, 0);
		if (!b) break;
		b = m_Drv.SetPixel(x, rect.Bottom, clLine, 0);
		if (!b) break;
	}
	if (!b) return false;

	for (y = rect.Top; y <= rect.Bottom; y++)
	{
		b = m_Drv.SetPixel(rect.Left, y, clLine, 0);
		if (!b) break;
		b = m_Drv.SetPixel(rect.Right, y, clLine, 0);
		if (!b) break;
	}
	if (!b) return false;

	return b;
}

bool Display::DrawCircle(int iX, int iY, int iRadius, COLORREF clBorder)
{
	int	v, w;
	int m;
	bool fRes = false;

	for (v = 0; v <= iRadius; v++) {
		m = (iRadius * iRadius) - (v * v);
		w = _Root2(m);

		/* Clock wise */
		fRes = m_Drv.SetPixel((iX + v), (iY - w), clBorder, 0);
		fRes = m_Drv.SetPixel((iX + w), (iY + v), clBorder, 0);
		fRes = m_Drv.SetPixel((iX - v), (iY + w), clBorder, 0);
		fRes = m_Drv.SetPixel((iX - w), (iY - v), clBorder, 0);

		/* Counter clock wise */
		fRes = m_Drv.SetPixel((iX - v), (iY - w), clBorder, 0);
		fRes = m_Drv.SetPixel((iX - w), (iY + v), clBorder, 0);
		fRes = m_Drv.SetPixel((iX + v), (iY + w), clBorder, 0);
		fRes = m_Drv.SetPixel((iX + w), (iY - v), clBorder, 0);
	}

	return fRes;
}

bool Display::DrawLine(int iXFrom, int iYFrom, int iXTo, int iYTo, COLORREF clLine)
{
	int dx,dy;
	bool b = false;

	dx = ((iXTo >= iXFrom) ? (iXTo - iXFrom) : (iXFrom - iXTo));
	dy = ((iYTo >= iYFrom) ? (iYTo - iYFrom) : (iYFrom - iYTo));

	int x,y,X,Y;
	x = 0;
	y = 0;
	
	do {
		if (dx >= dy) {
			y = (dy * x) / dx;
		} else {
			x = (dx * y) / dy;
		}

		X = ((iXTo >= iXFrom) ? (iXFrom + x) : (iXFrom - x));
		Y = ((iYTo >= iYFrom) ? (iYFrom + y) : (iYFrom - y));
		b = m_Drv.SetPixel(X, Y, clLine, 0);
		if (!b) break;

		if (dx >= dy) {
			if (x < dx) {
				x++;
			} else {
				break;
			}
		} else {
			if (y < dy) {
				y++;
			} else {
				break;
			}
		}
	} while(true);

	return b;
}

// Usefull when line direction is importance
bool Display::DrawLine(Point & ptFrom, Point & ptTo, COLORREF clLine)
{
	int	iDeltaX = 0;
	int	iDeltaY = 0;
	int	iMaxDelta;
	int	i;
	unsigned char uQuad = 0;
	int	iX, iY;
	bool fRes = false;

	if (ptFrom.x > ptTo.x) {
		iDeltaX = ptFrom.x - ptTo.x;
		uQuad |= NDISP__NEGATIVE_DELTA_X;
	} else {
		iDeltaX = ptTo.x - ptFrom.x;
		uQuad |= NDISP__POSITIVE_DELTA_X;
	}

	if (ptFrom.y > ptTo.y) {
		iDeltaY = ptFrom.y - ptTo.y;
		uQuad |= NDISP__NEGATIVE_DELTA_Y;
	} else {
		iDeltaY = ptTo.y - ptFrom.y;
		uQuad |= NDISP__POSITIVE_DELTA_Y;
	}

	iMaxDelta = (iDeltaX > iDeltaY) ? iDeltaX : iDeltaY;

	switch (uQuad)
	{
	case NDISP__QUADRAN_0:
		for (i = 0; i < iMaxDelta; i++) {
			if (iDeltaX > iDeltaY) {
				iX = ptFrom.x + i;
				iY = ptFrom.y + ((i * iDeltaY) / iDeltaX);
			} else {
				iY = ptFrom.y + i;
				if (iDeltaY > 0) {
					iX = ptFrom.x + ((i * iDeltaX) / iDeltaY);
				} else {
					iX = ptFrom.x;
				}
			}

			fRes = m_Drv.SetPixel(iX, iY, clLine, 0);
			if (!fRes) break;
		}
		break;


	case NDISP__QUADRAN_1:
		for (i = 0; i < iMaxDelta; i++) {
			if (iDeltaX > iDeltaY) {
				iX = ptFrom.x - i;
				iY = ptFrom.y + ((i * iDeltaY) / iDeltaX);
			} else {
				iY = ptFrom.y + i;
				if (iDeltaY > 0) {
					iX = ptFrom.x - ((i * iDeltaX) / iDeltaY);
				} else {
					iX = ptFrom.x;
				}
			}

			fRes = m_Drv.SetPixel(iX, iY, clLine, 0);
			if (!fRes) break;
		}
		break;


	case NDISP__QUADRAN_2:
		for (i = 0; i < iMaxDelta; i++) {
			if (iDeltaX > iDeltaY) {
				iX = ptFrom.x - i;
				iY = ptFrom.y - ((i * iDeltaY) / iDeltaX);
			} else {
				iY = ptFrom.y - i;
				if (iDeltaY > 0) {
					iX = ptFrom.x - ((i * iDeltaX) / iDeltaY);
				} else {
					iX = ptFrom.x;
				}
			}

			fRes = m_Drv.SetPixel(iX, iY, clLine, 0);
			if (!fRes) break;
		}
		break;


	case NDISP__QUADRAN_3:
		for (i = 0; i < iMaxDelta; i++) {
			if (iDeltaX > iDeltaY) {
				iX = ptFrom.x + i;
				iY = ptFrom.y - ((i * iDeltaY) / iDeltaX);
			} else {
				iY = ptFrom.y - i;
				if (iDeltaY > 0) {
					iX = ptFrom.x + ((i * iDeltaX) / iDeltaY);
				} else {
					iX = ptFrom.x;
				}
			}

			fRes = m_Drv.SetPixel(iX, iY, clLine, 0);
			if (!fRes) break;
		}
		break;


	default:
		break;
	}

	return fRes;
}

bool Display::SetTextColor(COLORREF clText, COLORREF clBackground, bool bTemporary)
{
	m_bTempTxtCol = bTemporary;
	if (!bTemporary) {
		m_clText	= clText;
		m_clTextBkg	= clBackground;
	} else {
		m_clPrevText	= m_clText;
		m_clPrevTextBkg	= m_clTextBkg;
		m_clText		= clText;
		m_clTextBkg		= clBackground;
	}
	return true;
}

bool Display::GetCharRect(NWindow::Rectangle & Rect)
{
	Rect.Left = 0;
	Rect.Top = 0;
	Rect.Right = m_Font.m_pFont->Width * m_iTextScale;
	Rect.Bottom = (m_Font.m_pFont->Height * m_iTextScale) + m_iLineSpace;
	return true;
}

bool Display::GetCharDim(int & iWidth, int & iHeight)
{
	iWidth = m_Font.m_pFont->Width * m_iTextScale;
	iHeight = (m_Font.m_pFont->Height * m_iTextScale) + m_iLineSpace;
	return true;
}

bool Display::DrawCaret(int iX, int iY, bool bVisible)
{
	return _DrawCaret(iX, iY, bVisible);
}

bool Display::DrawChar(char ch, int iX, int iY)
{
	return _DrawChar(ch, iX, iY, iX);
}

bool Display::DrawChar(char ch, int iX, int iY, int & iNextX)
{
	return _DrawChar(ch, iX, iY, iNextX);
}

bool Display::DrawChar(char ch, int iX, int iY, bool bCalcOnly, int iTextScale, int & iNextX)
{
	if (bCalcOnly) {
		iNextX = iX + (m_Font.m_pFont->Width * m_iTextScale);
		return true;
	}

	unsigned char *pCharData;
	unsigned char uc;
	bool b = false;
	
	b = m_Font.GetCharData(ch, &pCharData, 0);
	if (!b) return false;
	
	int	iMaxY = iY + m_Font.m_pFont->Height;
	int iMaxX = iX + m_Font.m_pFont->Width;
	int	iXPos = iX;
	int	iYPos = iY;

	for (int y = iY; y < iMaxY; y++)
	{
		uc = *pCharData++;

		for (int sy = 0; sy < iTextScale; sy++)
		{
			iXPos = iX;
			for (int x = iX; x < iMaxX; x++)
			{
				for (int sx = 0; sx < iTextScale; sx++)
				{
					if (uc & (0x80 >> (x - iX))) {
						b = m_Drv.SetPixel(iXPos, iYPos, m_clText, 0);
					} else {
						b = m_Drv.SetPixel(iXPos, iYPos, m_clTextBkg, 0);
					}
					if (!b) break;
					iXPos++;
				}
			}
			if (!b) break;
			iYPos++;
		}
	}

	if (b) iNextX = iXPos;

	return b;
}

bool Display::DrawEllipsis(int iX, int iY)
{
	int iDmy;
	return _DrawEllipsis(iX, iY, iDmy);
}

bool Display::WriteText(
						const char *szText, 
						char *pStartChar, 
						const NWindow::Rectangle &Rect, 
						Point *pLastPos,
						char **ppLastChar
						)
{
	Rectangle rcLine = Rect;

	char *p = const_cast<char *>(szText);
	char *pNext;
	bool b = false;

	if (!p) return false;

	int iStrLen, iStrWidth;
	int iH, iXMax;
	Point ptStart;
	iH = (m_Font.m_pFont->Height * m_iTextScale) + m_iLineSpace;

	if (pStartChar) {
		m_bTextCalc = true;
	} else {
		m_bTextCalc = false;
	}

	if (m_bMultiLine) {
		rcLine.Bottom = rcLine.Top + iH;

		while (*p != m_chNull)
		{
			if ((rcLine.Top + iH) > Rect.Bottom) {
				break;
			}

			b = _GetLineText(p, rcLine.Width(), &pNext, iStrLen, iStrWidth);
			if (!b) break;

			b = _CalcStartPos(rcLine, iStrWidth, ptStart, iXMax);
			if (!b) break;

			b = _WriteLineText(p, pStartChar, iStrLen, rcLine.Right, ptStart, &p);
			if (!b) break;

			rcLine.Offset(0, iH);
		}
	} else {
		b = _GetLineText(p, rcLine.Width(), &pNext, iStrLen, iStrWidth);
		if (b) {
			b = _CalcStartPos(rcLine, iStrWidth, ptStart, iXMax);
			if (b) {
				b = _WriteLineText(p, pStartChar, iStrLen, rcLine.Right, ptStart, &p);
			}
		}
	}

	if (b) {
		if (ppLastChar) *ppLastChar = p;
		if (pLastPos) *pLastPos = ptStart;
	}

	m_bTextCalc = false;

	if (m_bTempTxtCol)
		_RestoreTextColor();

	return b;
}

bool Display::WriteText(const char *szText, const NWindow::Rectangle &Rect)
{
	return WriteText(szText, 0, Rect, 0, 0);
}

bool Display::BeginWrite(const char *szText, const NWindow::Rectangle &Rect, bool bCalcOnly)
{
	if (!szText) return false;

	__p = const_cast<char *>(szText);
	if (!__p) return false;

	__iChH = (m_Font.m_pFont->Height * m_iTextScale) + m_iLineSpace;
	__iChW = m_Font.m_pFont->Width * m_iTextScale;
	__iDelBot = (2 * m_Font.m_pFont->Height * m_iTextScale) + m_iLineSpace;
	__rcText = Rect;
	__iXPos = Rect.Left;
	__iYPos = Rect.Top;
	__iWrapLen = 0;
	__pLine = 0;

	m_bTextCalc = bCalcOnly;

	return true;
}

bool Display::WriteNextChar(char **ppCur, char **ppNext, NWindow::Point * pPtNext, bool & bIsNewLine)
{
	if (!__p) return false;
	if (*__p == m_chNull) return false;

	if (ppCur) *ppCur = __p;

	bool b = false;

	bIsNewLine = false;

	if (*__p == m_chNL) 
	{
		bIsNewLine = true;

		if (m_bMultiLine) {
			if ((__iYPos + __iDelBot) > __rcText.Bottom) 
			{
				m_bTextCalc = true;
			} 

			_NextLine(__iXPos, __iYPos, __rcText.Left, __iChH);
			__p++;
			
			if (pPtNext) pPtNext->Set(__iXPos, __iYPos);

			return true;
		}
	}

	if ((__iXPos + __iChW) > __rcText.Right) 
	{
		if (m_bMultiLine) 
		{
			if (m_bWordWrap) 
			{
				__iWrapLen = _Wrap(
								__p, 
								__rcText.Left, 
								__rcText.Right, 
								__iXPos, 
								__iYPos, 
								__iChW, 
								&__p
								);
			} 
			if ((__iYPos + __iDelBot) > __rcText.Bottom) 
			{
				m_bTextCalc = true;
			} 
			bIsNewLine = true;
			_NextLine(__iXPos, __iYPos, __rcText.Left, __iChH);
		} 
		else 
		{
			m_bTextCalc = true;
		}
	}

	b = _DrawChar(*__p, __iXPos, __iYPos, __iXPos);

	if (b) {
		__p++;

		if (ppNext) *ppNext = __p;
		if (pPtNext) pPtNext->Set(__iXPos, __iYPos);
	}

	return b;
}

int Display::GetWrapLength()
{
	if (__iWrapLen < 0) return -1;
	return __iWrapLen;
}

void Display::EndWrite()
{
	if (__p) __p = 0;
}

};	// End of namespace NWindow
