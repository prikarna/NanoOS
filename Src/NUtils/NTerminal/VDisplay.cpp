/*
 * File    : VDisplay.cpp
 * Remark  : VDisplay class implementation.
 *
 */

#include "VDisplay.h"
#include "ResId.h"

/* List of VDisplay instances */
VDisplay * VDisplay::m_List[MAX_VDISPLAY] = {0};

/* Is VDisplay window class registered */
bool VDisplay::m_fIsRegistered = false;

/* C'tor */
VDisplay::VDisplay():
	m_Width(240),
	m_Height(320),
	m_hDc(NULL),
	m_hBmp(NULL),
	m_hWnd(NULL),
	m_szClassName(_T("VDisplayClass")),
	m_hDDC(NULL),
	m_hBrush(NULL)
{
	int	i;

	Handle.Set(this, 0, &VDisplay::_GetHandle);
	Title.Set(this, &VDisplay::_SetTitle, &VDisplay::_GetTitle);

	RtlZeroMemory(m_szTitle, sizeof(m_szTitle));

	for (i = 0; i < MAX_VDISPLAY; i++) {
		if (m_List[i] == 0) {
			m_List[i] = this;
			break;
		}
	}

	if (i == MAX_VDISPLAY)
		throw "Max. number of VDisplay instance exceeded!";
}

/* D'tor */
VDisplay::~VDisplay()
{
	if (m_hBrush) {
		DeleteObject(reinterpret_cast<HGDIOBJ>(m_hBrush));
	}
	for (int i = 0; i < MAX_VDISPLAY; i++) {
		if (m_List[i] == this) {
			m_List[i] = 0;
			break;
		}
	}
}

LRESULT CALLBACK VDisplay::_WindProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm)
{
	bool		bRes = false;

	for (int i = 0; i < MAX_VDISPLAY; i++)
	{
		if (VDisplay::m_List[i]) {
			if (VDisplay::m_List[i]->m_hWnd == hWnd) {
				bRes = VDisplay::m_List[i]->_Proc(uMsg, wParm, lParm);
				break;
			}
		}
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

	if (bRes) return static_cast<LRESULT>(FALSE);

	return DefMDIChildProc(hWnd, uMsg, wParm, lParm);
}

bool VDisplay::_Proc(UINT uMsg, WPARAM wParm, LPARAM lParm)
{
	bool		bRes = false;
	PAINTSTRUCT	ps;
	HDC			hDc;
	BOOL		fRes = FALSE;
	VDisplay::InputData		InDat;

	switch (uMsg)
	{
	case WM_CREATE:
		break;

	case WM_CLOSE:
		ShowWindow(m_hWnd, SW_MINIMIZE);
		bRes = true;
		break;

	case WM_DESTROY:
		if (m_hDDC) {
			ReleaseDC(m_hWnd, m_hDDC);
			m_hDDC = NULL;
		}
		if (m_hDc) {
			DeleteDC(m_hDc);
			m_hDc = NULL;
		}
		if (m_hBmp) {
			DeleteObject(reinterpret_cast<HGDIOBJ>(m_hBmp));
			m_hBmp = NULL;
		}
		m_hWnd = NULL;
		bRes = true;
		break;

	case WM_PAINT:
		hDc = BeginPaint(m_hWnd, &ps);
		if (hDc) {
			fRes = BitBlt(
						hDc,
						0,
						0,
						(ps.rcPaint.right - ps.rcPaint.left),
						(ps.rcPaint.bottom - ps.rcPaint.top),
						m_hDc,
						0,
						0,
						SRCCOPY
						);
			EndPaint(m_hWnd, &ps);
			bRes = true;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		InDat.Msg = uMsg;
		InDat.LParam = lParm;
		InDat.WParam = wParm;
		OnInput(&InDat);
		break;

	default:
		break;
	}

	return bRes;
}

bool VDisplay::Create(HINSTANCE hInst, HWND hClientWnd, const TCHAR *szTitle, int iLeft, int iTop, int iWidth, int iHeight)
{
	bool		bRes = false;
	HDC			hDc = NULL;
	HGDIOBJ		hSav = NULL;
	DWORD		dwStyles = 0;
	WNDCLASSEX	wcChild = {0};
	ATOM		at = 0;
	RECT		rc = {0};
	HANDLE		hImg = NULL;

	if (m_hWnd) return false;

	hImg = LoadImage(hInst, MAKEINTRESOURCE(DKICON_VDISP), IMAGE_ICON, 0, 0, (LR_DEFAULTSIZE | LR_VGACOLOR));

	if (!m_fIsRegistered) {
		wcChild.cbSize			= sizeof(WNDCLASSEX);
		wcChild.hbrBackground	= reinterpret_cast<HBRUSH>(COLOR_WINDOWFRAME);
		wcChild.hCursor			= LoadCursor(NULL, IDC_ARROW);
		if (hImg == NULL) {
			wcChild.hIcon		= LoadIcon(NULL, IDI_APPLICATION);
		} else {
			wcChild.hIcon		= static_cast<HICON>(hImg);
		}
		wcChild.hInstance		= hInst;
		wcChild.lpfnWndProc		= &VDisplay::_WindProc;
		wcChild.lpszClassName	= m_szClassName;
		wcChild.lpszMenuName	= NULL;

		at = RegisterClassEx(&wcChild);
		if (at <= 0) {
			return false;
		}
		
		m_fIsRegistered = true;
	}

	rc.left = iLeft;
	rc.top = iTop;
	rc.right = iLeft + iWidth;
	rc.bottom = iTop + iHeight;

	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW | MDIS_ALLCHILDSTYLES, FALSE, WS_EX_MDICHILD);

	do {
		m_hWnd = CreateWindowEx(
							WS_EX_MDICHILD,
							m_szClassName,
							szTitle,
							WS_CHILD | WS_VISIBLE | MDIS_ALLCHILDSTYLES,
							rc.left,
							rc.top,
							(rc.right - rc.left),
							(rc.bottom - rc.top),
							hClientWnd,
							NULL,
							hInst,
							NULL
							);
		if (!m_hWnd) break;

		dwStyles = (LONG_PTR) GetWindowLongPtr(m_hWnd, GWL_STYLE);
		dwStyles &= ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX);
		dwStyles = SetWindowLongPtr(m_hWnd, GWL_STYLE, dwStyles);

		hDc = GetDC(m_hWnd);
		if (!hDc) break;

		m_hDc = CreateCompatibleDC(hDc);
		if (!m_hDc) break;

		GetClientRect(m_hWnd, &rc);
		m_hBmp = CreateCompatibleBitmap(hDc, rc.right, rc.bottom);
		if (!m_hBmp) break;

		hSav = SelectObject(m_hDc, reinterpret_cast<HGDIOBJ>(m_hBmp));

		bRes = true;

	} while (FALSE);

	if (hDc) ReleaseDC(m_hWnd, hDc);

	if (!bRes) {
		if (m_hBmp) {
			DeleteObject(reinterpret_cast<HGDIOBJ>(m_hBmp));
			m_hBmp = NULL;
		}
		if (m_hDc) {
			DeleteDC(m_hDc);
			m_hDc = NULL;
		}
		if (m_hWnd) {
			DestroyWindow(m_hWnd);
			m_hWnd = NULL;
		}
	}

	return bRes;
}

bool VDisplay::SetPixel(int iX, int iY, COLORREF Color)
{
	COLORREF	cRes = 0;
	BOOL		fRes = FALSE;

	cRes = ::SetPixel(m_hDc, iX, iY, Color);
	if (cRes != 1) {
		fRes = InvalidateRect(m_hWnd, NULL, FALSE);
		if (fRes) {
			fRes = UpdateWindow(m_hWnd);
		}
	}

	return ((fRes) ? true : false);
}

bool VDisplay::GetPixel(int iX, int iY, COLORREF &Color)
{
	Color = ::GetPixel(m_hDc, iX, iY);
	return (Color != 1) ? true : false;
}

bool VDisplay::FillRectangle(const LPRECT pRectangle, COLORREF Color)
{
	BOOL	fRes = FALSE;
	RECT	rc = {0};

	if (m_hBrush) {
		DeleteObject(reinterpret_cast<HGDIOBJ>(m_hBrush));
	}

	m_hBrush = CreateSolidBrush(Color);
	if (!m_hBrush) return false;

	if (pRectangle == 0) {
		GetClientRect(m_hWnd, &rc);
	} else {
		CopyRect(&rc, pRectangle);
	}

	fRes = FillRect(m_hDc, &rc, m_hBrush);
	if (fRes) {
		fRes = InvalidateRect(m_hWnd, &rc, FALSE);
		if (fRes) {
			fRes = UpdateWindow(m_hWnd);
		}
	}

	return ((fRes) ? true : false);
}

bool VDisplay::Update()
{
	BOOL	fRes = FALSE;

	fRes = InvalidateRect(m_hWnd, NULL, TRUE);
	if (fRes) {
		fRes = UpdateWindow(m_hWnd);
	}

	return ((fRes) ? true : false);
}

HWND VDisplay::_GetHandle()
{
	return m_hWnd;
}

const TCHAR * VDisplay::_GetTitle()
{
	RtlZeroMemory(m_szTitle, sizeof(m_szTitle));
	int	iRes = 0;
	iRes = GetWindowText(m_hWnd, m_szTitle, sizeof(m_szTitle)/sizeof(TCHAR));
	if (iRes <= 0)
		return _T("");

	return &m_szTitle[0];
}

void VDisplay::_SetTitle(const TCHAR *szTitle)
{
	if (lstrlen(szTitle) > 0) {
		SetWindowText(m_hWnd, szTitle);
	} else {
		SetWindowText(m_hWnd, _T(""));
	}
}

/*
bool VDisplay::DirectFillRectangle(const LPRECT pRect, COLORREF Color)
{
	m_hDDC = GetDC(m_hWnd);
	if (!m_hDDC) return false;

	BOOL	fRes = FALSE;

	HBRUSH hBr = CreateSolidBrush(Color);
	if (!hBr) goto ExitFunct;

	fRes = FillRect(m_hDDC, pRect, hBr);

	DeleteObject(reinterpret_cast<HGDIOBJ>(hBr));

ExitFunct:
	ReleaseDC(m_hWnd, m_hDDC);

	return ((fRes) ? true : false);
}

bool VDisplay::DirectSetPixel(unsigned int X, unsigned int Y, COLORREF Color)
{
	COLORREF	cRes = 0;

	m_hDDC = GetDC(m_hWnd);
	if (!m_hDDC) return false;

	cRes = ::SetPixel(m_hDDC, X, Y, Color);

	ReleaseDC(m_hWnd, m_hDDC);

	return ((cRes != 1) ? true : false);
}

bool VDisplay::DirectUpdate()
{
	BOOL	fRes = InvalidateRect(m_hWnd, NULL, TRUE);
	return ((fRes) ? true : false);
}
*/