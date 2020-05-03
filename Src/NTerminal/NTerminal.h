/*
 * File    : NTerminal.h
 * Remark  : NTerminal class definition.
 *           A simple terminal for NanoOS.
 *
 */

#pragma once

#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"

#include "DkObject.h"

#define MAX_NTERM			4

class NTerminal : 
	public EventConsumer
{
private:
	HWND			m_hWnd;
	const TCHAR *	m_szClassName;
	static bool		m_fIsRegistered;
	HWND			m_hCtl;
	HFONT			m_hFont;
	ULONG			m_ulCharPos;
	TCHAR			m_szTitle[512];

	static NTerminal*	m_List[MAX_NTERM];

	static WNDPROC		m_DefProc;

	static LRESULT CALLBACK _EditHookProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm);
	static LRESULT CALLBACK _WindProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm);

	bool _HookProc(UINT uMsg, WPARAM wParm, LPARAM lParm);
	bool _Proc(UINT uMsg, WPARAM wParm, LPARAM lParm);

	HWND			_GetHandle() { return m_hWnd; }
	const TCHAR *	_GetTitle();
	void			_SetTitle(const TCHAR *szTitle);

public:
	NTerminal();
	~NTerminal();

/*
	Prop.		: Handle.
	Var. type	: HWND.
	Direction	: get.
	Desc.		: Handle to current window.
*/
	Property<NTerminal, HWND>			Handle;

/*
	Prop.		: Title.
	Var. type	: const TCHAR *. (string)
	Direction	: set and get.
	Desc.		: String of window text.
*/
	Property<NTerminal, const TCHAR *>	Title;

/*
	Event			: OnChar
	Desc.			: Triggered when a user type a char to terminal.
	Handler param.	: A char (char type).
*/
	Event<NTerminal, CHAR>							OnChar;

/*
	Funct.	: Create
	Desc.   : Create NTerminal.
	Params. :
		hInst
			Handle to instance of current process.
		hClientWnd
			Handle to MDI client window.
		szTitle
			String to be set to window text.
	Return  : true if success otherwise false.
*/
	bool Create(HINSTANCE hInst, HWND hClientWnd, const TCHAR *szTitle);

/*
	Funct.	: Printf
	Desc.   : Print string with specified format on terminal display.
	Params. :
		szFormat
			String format as StringCbVPrintf() string format.
		...
			List of arguments.
	Return  : None.
*/
	void Printf(const TCHAR *szFormat, ...);

/*
	Funct.	: AppendMultiByteText
	Desc.   : Append or add multibyte string to terminal display.
	Params. :
		szText
			Multibyte string or multibyte text to be addedd to current string or text on terminal.
	Return  : None.
*/
	void AppendMultiByteText(const char * szText);

/*
	Funct.	: AppendText
	Desc.   : Append or add string to terminal display.
	Params. :
		szText
			String or text to be addedd to current string or text on terminal diplay.
	Return  : None.
*/
	void AppendText(const TCHAR * szText);

/*
	Funct.	: AppendChar
	Desc.   : Append or add string to terminal display. (Not implemented yet).
	Params. :
		ch
			Character to be added current string or text on terminal display.
	Return  : None.
*/
	void AppendChar(char ch);

/*
	Funct.	: ClearText
	Desc.   : Clear current string or text on terminal display.
	Params. : None.
	Return  : None.
*/
	void ClearText();
};