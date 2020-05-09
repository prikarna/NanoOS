/*
 * File    : VDisplay.h
 * Remark  : VDisplay class definition.
 *           Simulate a LCD touch screen display to be used by NanoOS application.
 *
 */

#pragma once

#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"

#include "DkObject.h"

#define MAX_VDISPLAY		4

class VDisplay:
	public EventConsumer
{
public:
	struct InputData {
		UINT	Msg;
		LPARAM	LParam;
		WPARAM	WParam;
	};

private:
	HWND				m_hWnd;
	HDC					m_hDDC;
	const TCHAR *		m_szClassName;
	static bool			m_fIsRegistered;
	TCHAR				m_szTitle[512];

	int					m_Width;
	int					m_Height;
	HDC					m_hDc;
	HBITMAP				m_hBmp;
	HBRUSH				m_hBrush;

	static VDisplay *	m_List[MAX_VDISPLAY];

	static LRESULT CALLBACK _WindProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm);

	bool _Proc(UINT uMsg, WPARAM wParm, LPARAM lParm);

	HWND			_GetHandle();
	const TCHAR *	_GetTitle();
	void			_SetTitle(const TCHAR * szTitle);

public:
	VDisplay();
	~VDisplay();

/*
	Prop.		: Title.
	Var. type	: const TCHAR *. (string).
	Direction	: get.
	Desc.		: The string of window text.
*/
	Property<VDisplay, const TCHAR *>	Title;

/*
	Prop.		: Handle.
	Var. type	: HWND.
	Direction	: get.
	Desc.		: Handle to current window handle.
*/
	Property<VDisplay, HWND>			Handle;

/*
	Event			: OnInput
	Desc.			: Triggered when a mouse click on virtual display.
	Handler param.	: Pointer to VDisplay::InputData.
*/
	Event<VDisplay, VDisplay::InputData *>		OnInput;

/*
	Funct.	: Create
	Desc.   : Create vitual display.
	Params. :
		hInst
			Handle to instance of current process.
		hClientWnd
			Handle to MDI client window.
		szTitle
			Text to be displayed on child window.
		iLeft
			Left position of window.
		iTop
			Top position of window.
		iWidth
			Width of window.
		iHeight
			Height of window.
	Return  : true if success otherwise false.
*/
	bool Create(HINSTANCE hInst, HWND hClientWnd, const TCHAR * szTitle, int iLeft, int iTop, int iWidth, int iHeight);

/*
	Funct.	: SetPixel
	Desc.   : Set a pixel of virtual display.
	Params. :
		uiX
			X position of pixel from left of display client area.
		uiY
			Y position of pixel from top of display client area.
		Color
			Color of the pixel.
	Return  : true if success otherwise false.
*/
	bool SetPixel(unsigned int uiX, unsigned int uiY, COLORREF Color);

/*
	Funct.	: FillRectangle
	Desc.   : Draw or create filled rectangle on virtual display.
	Params. :
		pRectangle
			Pointer to RECT structure that rectangle to created.
		Color
			Color of the filled rectangle.
	Return  : true if success otherwise false.
*/
	bool FillRectangle(const LPRECT pRectangle, COLORREF Color);

/*
	Funct.	: Update
	Desc.   : Redraw virtual display buffer to virtual display.
	Params. : None.
	Return  : true if success otherwise false.
*/
	bool Update();

/*
	bool DirectSetPixel(unsigned int X, unsigned int Y, COLORREF Color);
	bool DirectFillRectangle(const LPRECT pRect, COLORREF Color);
	bool DirectUpdate();
*/
};
