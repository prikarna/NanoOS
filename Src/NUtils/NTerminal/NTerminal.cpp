/*
 * File    : NTerminal.cpp
 * Remark  : NTerminal implementation.
 *
 */

#include "NTerminal.h"
#include "ResId.h"

/* Is Window class already registered */
bool NTerminal::m_fIsRegistered = false;

/* Default WNDPROC of edit control */
WNDPROC NTerminal::m_DefProc = NULL;

/* List of this class instances */
NTerminal *	NTerminal::m_List[MAX_NTERM] = {0};

/* C'tor */
NTerminal::NTerminal():
	m_hWnd(NULL),
	m_szClassName(_T("NTerminalWindClass")),
	m_hCtl(NULL),
	m_hFont(NULL),
	m_ulCharPos(0)
{
	int	i;

	Handle.Set(this, 0, &NTerminal::_GetHandle);
	Title.Set(this, &NTerminal::_SetTitle, &NTerminal::_GetTitle);

	RtlZeroMemory(m_szTitle, sizeof(m_szTitle));

	for (i = 0; i < MAX_NTERM; i++) {
		if (NTerminal::m_List[i] == 0) {
			m_List[i] = this;
			break;
		}
	}
	if (i == MAX_NTERM) {
		throw "Number of object exeeded (Max. NTerminal = 4)!";
	}
}

/* D'tor */
NTerminal::~NTerminal()
{
	for (int i = 0; i < MAX_NTERM; i++) {
		if (NTerminal::m_List[i] == this) {
			m_List[i] = NULL;
			break;
		}
	}
}

bool NTerminal::Create(HINSTANCE hInst, HWND hClientWnd, const TCHAR *szTitle)
/*
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
{
	bool			fRes = false;
	RECT			rc = {0};
	WNDCLASSEX		wcx = {0};
	ATOM			at = 0;
	HANDLE			hImg = NULL;

	if (m_hWnd) return false;

	hImg = LoadImage(hInst, MAKEINTRESOURCE(DKICON_NTERM), IMAGE_ICON, 0, 0, (LR_DEFAULTSIZE | LR_VGACOLOR));

	if (!m_fIsRegistered) {
		wcx.cbSize			= sizeof(WNDCLASSEX);
		wcx.hbrBackground	= reinterpret_cast<HBRUSH>(COLOR_WINDOWTEXT);
		wcx.hCursor			= LoadCursor(NULL, IDC_ARROW);
		if (hImg == NULL) {
			wcx.hIcon		= LoadIcon(NULL, IDI_APPLICATION);
		} else {
			wcx.hIcon		= static_cast<HICON>(hImg);
		}
		wcx.hInstance		= hInst;
		wcx.lpfnWndProc		= reinterpret_cast<WNDPROC>(&NTerminal::_WindProc);
		wcx.lpszClassName	= m_szClassName;
		wcx.lpszMenuName	= NULL;

		at = RegisterClassEx(&wcx);
		if (at <= 0) return false;

		m_fIsRegistered = true;
	}

	SetRect(&rc, 0, 0, 600, 340);

	m_hWnd = CreateWindowEx(
							WS_EX_MDICHILD,
							m_szClassName,
							szTitle,
							WS_CHILD | WS_VISIBLE | MDIS_ALLCHILDSTYLES,
							rc.left,
							rc.right,
							(rc.right - rc.left),
							(rc.bottom - rc.top),
							hClientWnd,
							NULL,
							hInst,
							NULL
							);
	if (!m_hWnd) return false;

	LOGFONT		lf;
	LRESULT		lr;

	GetClientRect(m_hWnd, &rc);

	m_hCtl = CreateWindowEx(
							WS_EX_CLIENTEDGE,
							_T("EDIT"),
							NULL,
							WS_CHILD | WS_VISIBLE | WS_VSCROLL |
							ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
							rc.left, rc.top, rc.right, rc.bottom,
							m_hWnd,
							NULL,
							hInst,
							NULL
							);
	if (m_hCtl) {

		if (!m_hFont) {

			lf.lfCharSet			= DEFAULT_CHARSET;
			lf.lfClipPrecision		= CLIP_DEFAULT_PRECIS;
			lf.lfEscapement			= 0;
			StringCbCopy(lf.lfFaceName, sizeof(lf.lfFaceName), _T("Courier New"));
			lf.lfHeight				= -13;
			lf.lfItalic				= FALSE;
			lf.lfOrientation		= 0;
			lf.lfOutPrecision		= OUT_DEFAULT_PRECIS;
			lf.lfPitchAndFamily		= VARIABLE_PITCH | FF_SWISS;
			lf.lfQuality			= DEFAULT_QUALITY;
			lf.lfStrikeOut			= FALSE;
			lf.lfUnderline			= FALSE;
			lf.lfWeight				= FW_NORMAL;
			lf.lfWidth				= 0;

			m_hFont = CreateFontIndirect(&lf);
		}

		lr = SendMessage(m_hCtl, WM_SETFONT, (WPARAM) m_hFont, (LPARAM) TRUE);

		SendMessage(m_hCtl, EM_SETLIMITTEXT, (WPARAM) -1, 0);
		lr = SendMessage(m_hCtl, EM_GETLIMITTEXT, 0, 0);

		SendMessage(m_hCtl, EM_FMTLINES, (WPARAM) TRUE, 0);

		m_DefProc = reinterpret_cast<WNDPROC>(
			SetWindowLongPtr(m_hCtl, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(&NTerminal::_EditHookProc)));
	} else {
		return false;
	}

	return true;
}

LRESULT CALLBACK NTerminal::_EditHookProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm)
/*
	Desc.   : Edit control hook procedure. WNDPROC type.
	Params. :
		hWnd
			(Same as WNDPROC).
		uMsg
			(Same as WNDPROC).
		wParm
			(Same as WNDPROC).
		lParm
			(Same as WNDPROC).
	Return  : (Same as WNDPROC).
*/
{
	bool fRes = false;

	for (int i = 0; i < MAX_NTERM; i++) 
	{
		if (NTerminal::m_List[i]) 
		{
			if (NTerminal::m_List[i]->m_hCtl == hWnd) 
			{
				fRes = NTerminal::m_List[i]->_HookProc(uMsg, wParm, lParm);
				break;
			}
		}
	}

	if (!fRes) {
		return CallWindowProc(NTerminal::m_DefProc, hWnd, uMsg, wParm, lParm);
	}

	return static_cast<LRESULT>(FALSE);
}

LRESULT CALLBACK NTerminal::_WindProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm)
/*
	Desc.   : MDI child window procedure. WNDPROC type.
	Params. :
		hWnd
			(Same as WNDPROC).
		uMsg
			(Same as WNDPROC).
		wParm
			(Same as WNDPROC).
		lParm
			(Same as WNDPROC).
	Return  : (Same as WNDPROC).
*/
{
	static	UINT	uRef = 0;
	static	HBRUSH	hBrBg = NULL;
	bool			fRes = false;

	switch (uMsg)
	{
	case WM_CLOSE:
		ShowWindow(hWnd, SW_MINIMIZE);
		break;

	case WM_DESTROY:
		uRef++;
		if (uRef == 0) {
			DeleteObject(reinterpret_cast<HGDIOBJ>(hBrBg));
		}
		break;

	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC) wParm, RGB(0,255,0));
		SetBkColor((HDC) wParm, RGB(0,0,0));
		return (LRESULT) hBrBg;

	case WM_CREATE:
		if (uRef == 0) {
			hBrBg = CreateSolidBrush(RGB(0,0,0));
		}
		uRef++;
		break;

	default:
		for (int i = 0; i < MAX_NTERM; i++) {
			if (NTerminal::m_List[i])
			{
				if (NTerminal::m_List[i]->m_hWnd == hWnd)
				{
					fRes = NTerminal::m_List[i]->_Proc(uMsg, wParm, lParm);
					break;
				}
			}
		}
		if (!fRes) {
			return DefMDIChildProc(hWnd, uMsg, wParm, lParm);
		}
		break;
	}

	/* Must call DefMDIChildProc() for these 7 messages */
	if ((uMsg == WM_CHILDACTIVATE) || 
		(uMsg == WM_GETMINMAXINFO) ||
		(uMsg == WM_MENUCHAR) ||
		(uMsg == WM_MOVE) ||
		(uMsg == WM_SETFOCUS) ||
		(uMsg == WM_SIZE) ||
		(uMsg == WM_SYSCOMMAND))
	{
		return DefMDIChildProc(hWnd, uMsg, wParm, lParm);
	}

	return (LRESULT) FALSE;
}

bool NTerminal::_Proc(UINT uMsg, WPARAM wParm, LPARAM lParm)
/*
	Desc.   : Window procedure (MDI child window procedure) handler of current instance.
	Params. :
		uMsg
			(Same as WNDPROC).
		wParm
			(Same as WNDPROC).
		lParm
			(Same as WNDPROC).
	Return  : true if window message is handled or otherwise false.
*/
{
	bool	fRes = false;

	switch (uMsg)
	{
	case WM_SIZE:
		MoveWindow(m_hCtl, 0, 0, static_cast<int>(LOWORD(lParm)), static_cast<int>(HIWORD(lParm)), TRUE);
		fRes = true;
		break;

	default:
		break;
	}

	return fRes;
}

bool NTerminal::_HookProc(UINT uMsg, WPARAM wParm, LPARAM lParm)
/*
	Desc.   : Edit control hook procedure handler of current instance.
	Params. :
		uMsg
			(Same as WNDPROC).
		wParm
			(Same as WNDPROC).
		lParm
			(Same as WNDPROC).
	Return  : true if window message is handled or otherwise false.
*/
{
	bool	fRes = false;

	switch (uMsg)
	{
	case WM_CHAR:
		OnChar(static_cast<CHAR>(wParm));
		break;

	default:
		break;
	}

	return fRes;
}

void NTerminal::Printf(const TCHAR *szFormat, ...)
/*
	Desc.   : Print string with specified format on terminal.
	Params. :
		szFormat
			String format as StringCbVPrintf() string format.
		...
			List of arguments.
	Return  : None.
*/
{
	TCHAR	szBuf[1024] = {0};
	va_list	al;
	HRESULT	hr;

	va_start(al, szFormat);
	hr = StringCbVPrintf(szBuf, sizeof(szBuf), szFormat, al);
	va_end(al);

	SendMessage(m_hCtl, EM_SETSEL, (WPARAM) m_ulCharPos, (LPARAM)(m_ulCharPos + 1));
	SendMessage(m_hCtl, EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) szBuf);

	m_ulCharPos += lstrlen(szBuf);
}

void NTerminal::AppendMultiByteText(const char *szText)
/*
	Desc.   : Append or add multibyte string to terminal.
	Params. :
		szText
			Multibyte string or multibyte text to be addedd to current string or text on terminal.
	Return  : None.
*/
{
	SendMessageA(m_hCtl, EM_SETSEL, (WPARAM) m_ulCharPos, (LPARAM)(m_ulCharPos + 4));
	SendMessageA(m_hCtl, EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) szText);
	m_ulCharPos += lstrlenA(szText);
}

void NTerminal::AppendText(const TCHAR * szText)
/*
	Desc.   : Append or add string to terminal.
	Params. :
		szText
			String or text to be addedd to current string or text on terminal.
	Return  : None.
*/
{
	SendMessage(m_hCtl, EM_SETSEL, (WPARAM) m_ulCharPos, (LPARAM)(m_ulCharPos + 4));
	SendMessage(m_hCtl, EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) szText);
	m_ulCharPos += lstrlen(szText);
}

void NTerminal::ClearText()
/*
	Desc.   : Clear current string or text on terminal.
	Params. : None.
	Return  : None.
*/
{
	SetWindowText(m_hCtl, _T(""));
	m_ulCharPos = 0;
}

void NTerminal::_SetTitle(const TCHAR *szTitle)
/*
	Desc.   : Setter function of Title property. String or text to be displayed in terminal window.
	Params. : 
		szTitle
			String or text to be displayed in terminal window.
	Return  : None.
*/
{
	if (lstrlen(szTitle) > 0) {
		SetWindowText(m_hWnd, szTitle);
	} else {
		SetWindowText(m_hWnd, _T(""));
	}
}

const TCHAR * NTerminal::_GetTitle()
/*
	Desc.   : Getter function of Title property. String or text to be displayed in terminal window.
	Params. : None.
	Return  : String or text to be displayed in terminal window.
*/
{
	int	iRes = 0;

	RtlZeroMemory(m_szTitle, sizeof(m_szTitle));
	iRes = GetWindowText(m_hWnd, m_szTitle, sizeof(m_szTitle)/sizeof(TCHAR));
	if (iRes < 0)
		return _T("");

	return &m_szTitle[0];
}
