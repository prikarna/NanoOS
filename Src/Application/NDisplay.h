/*
 * File    : NDisplay.h
 * Remark  : Display and other related class definition.
 *           Provide basic infrastructure to create GUI application, for now only works on VDisplay
 *           which available in NTerminal.
 *
 */

#pragma once

#include "VDispData.h"
#include "NanoStd.h"

#define COLORREF		VDISP_COLORREF
#define RGB(r,g,b)		VDISP_RGB(r,g,b)

namespace NWindow
{

/*
 * Driver class: To access VDisplay in NTerminal
 */
class Driver
{
	bool				m_bEnabled;
	VDISP_OUT_DATA		m_OutDat;

public:
	Driver();
	~Driver();

	bool Initialize();
	void UnInitialize();
	bool SetPixel(int iX, int iY, COLORREF col, COLORREF *pRetCol);
	bool GetPixel(int iX, int iY, COLORREF &retCol);
	bool Fill(COLORREF col);
	bool Fill(COLORREF col, VDISP_RECTANGLE &Rect);
	bool GetRect(VDISP_RECTANGLE &Rect);
};

/* Simple font structure */
extern "C" {
#pragma pack(push)
#pragma pack(1)
	typedef struct _FONT {
		unsigned char	Width;
		unsigned char	Height;
		unsigned char	NumbOfChars;
		unsigned char	StartingChar;
		unsigned char	Data[1];
	} FONT, *PFONT;
#pragma pack(pop)
};

class Display;

/*
 * Font class: A Simple font
 */
class Font
{
	static const unsigned char	m_Data[1156];
	PFONT						m_pFont;

public:
	Font();
	~Font();

	bool Create(PFONT pFont);
	void Destroy();
	int CharWidth();
	int CharHeight();
	int NumberOfChars();
	bool GetCharData(char ch, unsigned char **ppData, int *piDataLength);
	bool GetFirstChar(unsigned char **ppData);

	friend class Display;
};

struct Rectangle
{
	int Left;
	int Top;
	int Right;
	int Bottom;

	Rectangle();
	~Rectangle() {};

	Rectangle operator =(Rectangle RightRect);		// Copy
	bool operator ==(Rectangle RightRect);			// Is Equal
	bool operator !=(Rectangle RightRect);			// Is Not Equal
	Rectangle operator +=(int Value);				// Inflate, can be negative value
	Rectangle operator &=(Rectangle RightRect);		// Intersection
	Rectangle operator |=(Rectangle RightRect);		// Union

	Rectangle operator =(VDISP_RECTANGLE RightRect);	// Convert from
	operator VDISP_RECTANGLE();							// Convert to

	bool IsEmpty();
	void Reset();
	void Set(int iLeft, int iTop, int iRight, int iBottom);
	void Offset(int idx, int idy);
	int Width();
	int Height();
};

struct Point
{
	int		x;
	int		y;

	Point() 
	{
		x = 0;
		y = 0;
	}

	~Point() {}

	Point operator =(Point RightPoint)
	{
		x = RightPoint.x;
		y = RightPoint.y;
		return *this;
	}

	Point operator +=(int Value)
	{
		x += Value;
		y += Value;
		return *this;
	}

	//Point operator +(Point RightPoint)
	//{
	//	x + RightPoint.x;
	//	y + RightPoint.y;
	//	return *this;
	//}

	//Point operator -(Point RightPoint)
	//{
	//	x - RightPoint.x;
	//	y - RightPoint.y;
	//	return *this;
	//}

	bool operator ==(Point RightPoint)
	{
		if ((x == RightPoint.x) &&
			(y == RightPoint.y))
		{
			return true;
		}
		return false;
	}

	bool operator !=(Point RightPoint)
	{
		if ((x != RightPoint.x) ||
			(y != RightPoint.y))
		{
			return true;
		}
		return false;
	}

	void Set(int ix, int iy)
	{
		x = ix;
		y = iy;
	}

	bool IsInRect(Rectangle &Rect)
	{
		if ((x > Rect.Left) &&
			(x < Rect.Right) &&
			(y > Rect.Top) &&
			(y < Rect.Bottom))
		{
			return true;
		}
		return false;
	}

	bool IsInRect(Rectangle & Rect, bool bIncLine)
	{
		if (bIncLine) {
			if ((x >= Rect.Left) &&
				(x <= Rect.Right) &&
				(y <= Rect.Bottom) &&
				(y >= Rect.Top))
			{
				return true;
			}
		} else {
			if ((x > Rect.Left) &&
				(x < Rect.Right) &&
				(y > Rect.Top) &&
				(y < Rect.Bottom))
			{
				return true;
			}
		}
		return false;
	}

	bool IsXInRange(int iMin, int iMax)
	{
		if ((x >= iMin) && (x <= iMax))
			return true;

		return false;
	}

	bool IsYInRange(int iMin, int iMax)
	{
		if ((y >= iMin) && (y <= iMax))
			return true;

		return false;
	}
};

struct TextAlign
{
	enum E {
		LeftTop,
		TopRight,
		Center,
		TopHCenter,
		BottomHCenter,
		LeftVCenter,
		RightVCenter
	};
};

/*
 * Display class: To draw an object. Most of this functions are just pixel 
 *                operation and don't care about pixel size.
 */
class Display
{
	Driver		m_Drv;
	Font		m_Font;
	COLORREF	m_clText;
	COLORREF	m_clTextBkg;
	COLORREF	m_clPrevText;
	COLORREF	m_clPrevTextBkg;
	bool		m_bTempTxtCol;
	bool		m_bTextCalc;

	/*
	 * Vars. for write text by writing char by char, starting with BeginWrite()
	 */
	Rectangle	__rcText;
	int			__iChW;
	int			__iChH;
	int			__iDelBot;
	int			__iXPos;
	int			__iYPos;
	char *		__p;
	int			__iWrapLen;
	char *		__pLine;

	unsigned int _Root2(unsigned int uValue);

	void _RestoreTextColor();
	bool _GetWordWidth(const char *szWord, bool bIncDelim, int iMaxWidth, int &iWordLength, int &iWidth);
	bool _GetLineText(char *pStart, int iMaxWidth, char **ppNext, int & iStrLen, int & iStrWidth);
	bool _DrawChar(char ch, int iX, int iY);
	bool _DrawChar(char ch, int iX, int iY, int & iNextX);
	bool _DrawEllipsis(int iX, int iY, int &iNextX);
	bool _DrawCaret(int iX, int iY, bool bVisible);
	bool _CalcStartPos(Rectangle & rcLine, int iStringWidth, Point & ptRes, int & iXMaxRes);
	bool _WriteLineText(
				char *pLine, 
				char *pStartChar, 
				int iStringLength, 
				int iLineRight, 
				Point & ptWrite,
				char **ppNext
				);

	bool _NextLine(int & iCurX, int & iCurY, int iLeft, int iCharHeight);
	int _Wrap(char *pCur, int iLeft, int iRight, int iCurX, int iCurY, int iCharWidth, char **ppNext);

public:
	int				m_iTextScale;
	char			m_chWordDelim;
	char			m_chNL;			// New line char.
	char			m_chNull;		// NULL char., to identify end of string
	int				m_iLineSpace;
	TextAlign::E	m_eTextAlign;

	bool			m_bEndEllipsis;
	bool			m_bWordWrap;
	bool			m_bMultiLine;

	Display(void);
	~Display(void);

	bool Create();
	void Destroy();

	bool GetRect(Rectangle &rect);

	bool SetPixel(int iX, int iY, COLORREF col, COLORREF *pRetCol);
	bool GetPixel(int iX, int iY, COLORREF &clRes);

	bool Fill(COLORREF col);
	bool Fill(COLORREF col, Rectangle &rect);
	bool DrawLine(int iXFrom, int iYFrom, int iXTo, int iYTo, COLORREF clLine);
	bool DrawLine(Point & ptFrom, Point & ptTo, COLORREF clLine);	// When line direction is important
	bool DrawRect(int iX, int iY, int iWidth, int iHeigth, COLORREF clLine);
	bool DrawRect(Rectangle &rect, COLORREF clLine);
	bool DrawCircle(int iX, int iY, int iRadius, COLORREF clBorder);

	bool SetTextColor(COLORREF clText, COLORREF clBackground, bool bTemporary);

	bool GetCharDim(int & iWidth, int & iHeight);
	bool GetCharRect(Rectangle & rect);
	bool DrawCaret(int iX, int iY, bool bVisible);

	bool DrawChar(char ch, int iX, int iY);
	bool DrawChar(char ch, int iX, int iY, int & iNextX);
	bool DrawChar(char ch, int iX, int iY, bool bCalcOnly, int iTextScale, int & iNextX);
	bool DrawEllipsis(int iX, int iY);

	bool WriteText(
			const char *szText, 
			char *pStartChar, 
			const Rectangle & Rect, 
			Point *pLastPos,
			char **ppLastChar
			);
	bool WriteText(const char *szText, const Rectangle & Rect);

	bool BeginWrite(const char *szText, const Rectangle &Rect, bool bCalcOnly);
	bool WriteNextChar(char **ppCur, char **ppNext, Point *pPtNext, bool & bIsNewLine);
	int GetWrapLength();
	void EndWrite();
};

};	// End of namespace NWindow
