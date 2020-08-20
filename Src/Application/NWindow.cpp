#include "NWindow.h"

namespace NWindow
{

Base *						Base::m_pCurWind = 0;
NanoStd::List<Base *, 4>	Base::m_List;
bool						Base::m_bStopDispInput;
unsigned int				Base::m_uiInTID = 0;

Base::Base():
m_clBlack(RGB(0,0,0)),
m_clWhite(RGB(255,255,255))
{
	m_clGr64	= RGB(64,64,64);
	m_clGr128	= RGB(128,128,128);
	m_clGr192	= RGB(192,192,192);
}

Base::~Base()
{
}

int Base::_DispInputLoop(void *pDat)
{
	BOOL fRes = FALSE;
	bool b = false;
	VDISP_IN_DATA InDat;
	Input Inp;

	while (!m_bStopDispInput)
	{
		fRes = ReadFromUsbSerial(reinterpret_cast<UINT8_PTR_T>(&InDat), sizeof(VDISP_IN_DATA), 0);
		if (fRes) {
			switch (InDat.Type)
			{
			case VDISP_IN_TYPE__TOUCH:
				Inp.Type = InputType::Touched;
				Inp.FromPoint(InDat.X, InDat.Y);
				b = true;
				break;

			case VDISP_IN_TYPE__RELEASED:
				Inp.Type = InputType::Released;
				Inp.FromPoint(InDat.X, InDat.Y);
				b = true;
				break;

			default:
				b = false;
				DBG_PRINTF("%s: Unknown VDisplay input type: %d\r\n", __FUNCTION__, InDat.Type);
				break;
			}

			if (b) {
				b = PutInput(Inp);
			}
		}
	}

	return 0;
}

bool Base::Select(Base * pWind)
{
	if (pWind == m_pCurWind) return true;

	if (m_pCurWind) {
		m_pCurWind->m_Disp.Destroy();
		m_pCurWind = 0;
	}

	if (!pWind) return false;

	bool b = pWind->m_Disp.Create();
	if (b)
		m_pCurWind = pWind;

	return b;
}

bool Base::IsCurrent()
{
	return ((m_pCurWind == this) ? true : false);
}

bool Base::Create()
{
	bool b = false;

	b = m_List.Find(this);
	if (b) return true;

	if (!m_Disp.Create()) return false;

	m_InQue.Reset();

	b = m_List.PushBack(this);
	if (b) {
		OnCreate();
	}
	return b;
}

void Base::Destroy()
{
	Base * pWind = 0;
	bool b = false;

	b = m_List.Find(this);
	if (b) {
		OnDestroy();

		if (m_pCurWind == this) {
			m_pCurWind = 0;
		}
		m_List.Remove(this);
		m_List.Last(pWind);
		if (pWind) {
			pWind->Show();
		}
	}
}

bool Base::PutInput(Input & Inp)
{
	if (!m_pCurWind) return false;
	return m_pCurWind->m_InQue.Put(Inp);
}

bool Base::GetInput(Input & Inp)
{
	return m_InQue.Get(Inp);
}

bool Base::StartDisplayInput()
{
	if (IsUsbSerialReady() == FALSE) return false;

	m_bStopDispInput = false;
	BOOL fRes = FALSE;
	fRes = CreateThread(
				THREAD_CONTROL__UNPRIVILEGED, 
				FALSE, 
				reinterpret_cast<THREAD_ENTRY_TYPE>(Base::_DispInputLoop), 
				0, 
				&m_uiInTID
				);
	if (!fRes) m_bStopDispInput = true;
	return (fRes) ? true : false;
}

bool Base::StopDisplayInput()
{
	if (m_bStopDispInput) return true;

	m_bStopDispInput = true;
	CancelReadFromUsbSerial();
	Sleep(100);

	UINT8_T uStat = 0;
	GetThreadStatus(m_uiInTID, &uStat);
	if ((uStat == THREAD_STATUS__EXIT) || (uStat == THREAD_STATUS__TERMINATED))
	{
		return true;
	}

	TerminateThread(m_uiInTID);
	CancelReadFromUsbSerial();

	return true;
}

Screen::Screen():
m_bEnaIconContainer(false),
m_bIconTouched(false),
m_ciIconWidth(80),
m_ciIconHeight(110)
{
	IconContainer.Set(this, &Screen::_SetEnaIconContainer, &Screen::_GetEnaIconContainer);
}

Screen::~Screen()
{
}

bool Screen::_ShowIcons()
{
	Rectangle rc;
	Int<unsigned int> Id;
	Icon * pIcon;

	m_Grid.m_Rows[0].m_Columns[0].m_bEnable = true;
	m_Grid.m_Rows[0].m_Columns[0].m_szName = "Info";
	m_Grid.m_Rows[0].m_Columns[1].m_bEnable = true;
	m_Grid.m_Rows[0].m_Columns[1].m_szName = "GPIO";

	rc.Set(0, 0, m_ciIconWidth, m_ciIconHeight);
	m_Disp.SetTextColor(m_clBlack, m_clWhite, false);
	m_Disp.m_eTextAlign = TextAlign::TopHCenter;
	m_Disp.m_iTextScale = 1;
	m_Disp.m_iLineSpace = 2;
	m_Disp.m_bMultiLine = true;
	m_Disp.m_bWordWrap = true;

	for (unsigned int r = 0; r < m_Grid.m_Rows.Size(); r++)
	{
		for (unsigned int c = 0; c < m_Grid.m_Rows[r].m_Columns.Size(); c++)
		{
			pIcon = &m_Grid.m_Rows[r].m_Columns[c];
			pIcon->m_Id = Id;
			pIcon->m_rc = rc;
			pIcon->m_rc += -5;
			pIcon->m_rc.Bottom += 5;
			pIcon->m_rcImg = rc;
			pIcon->m_rcImg += -18;
			pIcon->m_rcImg.Bottom = pIcon->m_rcImg.Top + pIcon->m_rcImg.Width();
			pIcon->m_rcText = rc;
			pIcon->m_rcText += -10;
			pIcon->m_rcText.Top = pIcon->m_rcImg.Bottom + 10;

			rc.Offset(m_ciIconWidth, 0);

			if (pIcon->m_bEnable) {
				m_Disp.WriteText(pIcon->m_szName.CStr(), pIcon->m_rcText);
				m_Disp.DrawRect(pIcon->m_rcImg, m_clBlack);
			}

			Id++;
		}
		rc.Left = 0;
		rc.Right = m_ciIconWidth;
		rc.Offset(0, m_ciIconHeight);
	}

	return true;
}

/*
 * For testing the display
 *
#ifdef _DEBUG
void Screen::_TestDisplay()
{
	Rectangle rc;
	const char *szT = 
		"Kambing liar melompat dari pagar tetangga untuk mencari rumput di lapangan bola. "
		"Seseorang melihatnya dan mencoba mencegah kambing itu makan rumput di lapangan tersebut. "
		"Namun kambing lari dan orang tersebut mencoba mengejarnya sampai kelelahan dan akhirnya menyerah.";

	const char *szT2 = "The NanoOS.";
	char *pStart = const_cast<char *>(szT);
	pStart += 42;

	rc.Set(5, 5, 200, 100);
	m_Disp.m_bMultiLine = false;
	m_Disp.m_bWordWrap = false;
	m_Disp.m_bEndEllipsis = false;
	m_Disp.m_eTextAlign = TextAlign::Center;
	m_Disp.WriteText(szT2, rc);

	m_Disp.DrawRect(rc, m_clBlack);
}
#endif
*/

bool Screen::_ShowScreen()
{
	Rectangle rc;
	rc = m_rcDisp;

	rc += -24;
	rc.Top += 68;
	rc.Bottom -= 126;
	m_Disp.Fill(m_clBlack, rc);
	m_Disp.SetTextColor(m_clWhite, m_clBlack, true);
	m_Disp.m_bEndEllipsis = false;
	m_Disp.m_bMultiLine = false;
	m_Disp.m_eTextAlign = TextAlign::Center;
	m_Disp.m_iTextScale = 3;
	m_Disp.WriteText("NanoOS", 0, rc, 0, 0);
	rc += -6;
	m_Disp.DrawRect(rc, m_clWhite);

	//_TestDisplay();
	
	return true;
}

bool Screen::_DrawIcon(Icon * pCurIcon)
{
	bool b = false;
	Rectangle rc;

	if (!pCurIcon) return false;
	rc = pCurIcon->m_rcImg;

	if (m_bIconTouched) {
		m_Disp.Fill(m_clBlack, rc);
		m_Disp.DrawRect(pCurIcon->m_rc, m_clBlack);
	} else {
		m_Disp.DrawRect(pCurIcon->m_rc, m_clWhite);
		m_Disp.Fill(m_clWhite, rc);
		m_Disp.DrawRect(rc, m_clBlack);
	}

	return b;
}

bool Screen::_DefInputHandler(Input & Inp)
{
	bool b = false;
	Point pt;

	switch (Inp.Type)
	{
	case InputType::Touched:
		Inp.ToPoint(pt);
		OnTouched(pt);
		b = true;
		break;

	case InputType::Released:
		if (m_bIconTouched) {
			m_bIconTouched = false;
			_DrawIcon(m_pSelIcon);
		}
		Inp.ToPoint(pt);
		OnReleased(pt);
		b = true;
		break;

	default:
		break;
	}

	return b;
}

void Screen::_SetEnaIconContainer(bool bEnable)
{
	if (m_bEnaIconContainer != bEnable) {
		m_bEnaIconContainer = bEnable;
		if (IsCurrent())
			Show();
	}
}

bool Screen::_GetEnaIconContainer()
{
	return m_bEnaIconContainer;
}

bool Screen::Show()
{
	bool b = Select(dynamic_cast<Base *>(this));
	if (!b) return false;

	m_Disp.Fill(m_clWhite);
	m_Disp.GetRect(m_rcDisp);

	if (m_bEnaIconContainer) {
		_ShowIcons();
	} else {
		_ShowScreen();
	}

	OnShow();
	
	return true;
}

bool Screen::ProcessInput(Input & Inp)
{
	bool	b = false;
	bool	bInIcon = false;
	Icon *	pSelIcon = 0;
	unsigned int r,c;
	Point	pt;

	if (m_bEnaIconContainer) 
	{
		for (r = 0; r < m_Grid.m_Rows.Size(); r++)
		{
			for (c = 0; c < m_Grid.m_Rows[r].m_Columns.Size(); c++)
			{
				pSelIcon = &m_Grid.m_Rows[r].m_Columns[c];
				Inp.ToPoint(pt);
				bInIcon = pt.IsInRect(pSelIcon->m_rc);
				if (bInIcon) {
					if (pSelIcon->m_bEnable == false)
					{
						pSelIcon = 0;
					}
					break;
				}
			}
			if (bInIcon) break;
		}

		if (bInIcon)
		{
			if (Inp.Type == InputType::Touched)
			{
				m_pSelIcon = pSelIcon;
				m_bIconTouched = true;
			}
			else
			{
				if (m_pSelIcon == pSelIcon)
				{
					if (pSelIcon) {
						OnIconTouched(pSelIcon->m_Id);
					}
				}
				else
				{
					pSelIcon = m_pSelIcon;
				}
				m_bIconTouched = false;
			}
			_DrawIcon(pSelIcon);
			if (pSelIcon == 0)
				_DefInputHandler(Inp);
		}
		else
		{
			b = _DefInputHandler(Inp);
		}
	}
	else
	{
		b = _DefInputHandler(Inp);
	}

	return b;
}

Window::Window(void):
m_ciHdrHeight(20),
m_bIconTouched(false),
m_bCloseTouched(false),
m_bIsShown(false)
{
	m_strTitle = "NanoWindow";
	Text.Set(this, &Window::_SetText, &Window::_GetText);
}

Window::~Window(void)
{
}

bool Window::_DrawHeader()
{
	m_Disp.Fill(m_clBlack, m_rcHeader);

	// Draw window icon
	m_rcIcon = m_rcHeader;
	m_rcIcon += -3;
	m_rcIcon.Right = m_rcIcon.Left + m_rcIcon.Height();
	m_Disp.Fill(m_clWhite, m_rcIcon);
	m_Disp.DrawChar('N', m_rcIcon.Left + 4, m_rcIcon.Top + 2);

	// Write title;
	Rectangle rc;
	rc = m_rcHeader;
	rc.Left += m_rcIcon.Width();
	rc.Left += 8;
	rc.Top += 4;
	m_Disp.SetTextColor(m_clWhite, m_clBlack, true);
	m_Disp.m_iTextScale = 1;
	m_Disp.m_eTextAlign = TextAlign::LeftTop;
	m_Disp.m_bMultiLine = false;
	m_Disp.WriteText(m_strTitle.CStr(), rc);

	_DrawCloseButon(false);

	return true;
}

bool Window::_DrawCloseButon(bool bIsTouched)
{
	COLORREF	clFillCol, clLineCol;

	if (bIsTouched) {
		clFillCol = m_clBlack;
		clLineCol = m_clWhite;
	} else {
		clFillCol = m_clWhite;
		clLineCol = m_clBlack;
	}

	m_Disp.Fill(clFillCol, m_rcClose);
	Rectangle rc = m_rcClose;
	rc += -3;
	m_Disp.DrawLine((rc.Left+1), rc.Top, rc.Right, (rc.Bottom-1), clLineCol);
	m_Disp.DrawLine(rc.Left, rc.Top, rc.Right, rc.Bottom, clLineCol);
	m_Disp.DrawLine(rc.Left, (rc.Top+1), (rc.Right-1), rc.Bottom, clLineCol);

	m_Disp.DrawLine(rc.Left, (rc.Bottom-1), (rc.Right-1), rc.Top, clLineCol);
	m_Disp.DrawLine(rc.Left, rc.Bottom, rc.Right, rc.Top, clLineCol);
	m_Disp.DrawLine((rc.Left+1), rc.Bottom, rc.Right, (rc.Top+1), clLineCol);

	return true;
}

void Window::_SetText(const char *szText)
{
	m_strTitle = szText;

	if (!m_bIsShown) return;

	m_Disp.Fill(m_clBlack, m_rcHeader);
	_DrawHeader();
}

const char * Window::_GetText()
{
	return m_strTitle.CStr();
}

bool Window::_HandleTouchRelease(NWindow::Input &Inp)
{
	bool		b = false;
	bool		bCanClose;
	Control *	pCtl = 0;
	Point		pt;

	Inp.ToPoint(pt);

	if (pt.IsInRect(m_rcIcon))
	{
		if (Inp.Type == InputType::Touched) {
			m_bIconTouched = true;
		} else {
			if (m_bIconTouched) {
				m_bIconTouched = false;
				OnIconTouched();
			}
		}
	}
	else if (pt.IsInRect(m_rcClose))
	{
		if (Inp.Type == InputType::Touched) {
			m_bCloseTouched = true;
			_DrawCloseButon(true);
		} else {
			_DrawCloseButon(false);

			if (m_bCloseTouched) {
				m_bCloseTouched = false;
				bCanClose = true;
				OnClose(bCanClose);
				if (bCanClose) {
					Destroy();
				}
			}
		}
	}
	else if (pt.IsInRect(m_rcHeader))
	{
		// Reserved
	}
	else
	{
		b = false;
		m_CtlIt = m_CtlList.Begin();
		while (m_CtlIt != m_CtlList.End())
		{
			b = pt.IsInRect((*m_CtlIt)->m_rc);
			if (b) {
				if ((*m_CtlIt)->m_bVisible) {
					if (pCtl == 0)
						pCtl = *m_CtlIt;
				}
			} else {
				(*m_CtlIt)->m_bFocused = false;
			}
			m_CtlIt++;
		}
		if (pCtl) {
			pCtl->m_bFocused = true;
			pCtl->_ProcessInput(Inp);
		} else {
			switch (Inp.Type)
			{
			case InputType::Touched:
				OnTouched(pt);
				break;

			case InputType::Released:
				OnReleased(pt);
				break;

			default:
				break;
			}
		}
	}

	return b;
}

bool Window::_HandleChar(NWindow::Input &Inp)
{
	bool b = false;

	m_CtlIt = m_CtlList.Begin();
	while (m_CtlIt != m_CtlList.End())
	{
		if ((*m_CtlIt)->m_bVisible) {
			if ((*m_CtlIt)->m_bFocused)
			{
				(*m_CtlIt)->_ProcessInput(Inp);
				b = true;
				break;
			}
		}
		m_CtlIt++;
	}

	return b;
}

void Window::Destroy()
{
	if (m_CtlList.Count() > 0) {
		m_CtlIt = m_CtlList.Begin();
		while (m_CtlIt != m_CtlList.End())
		{
			DeregisterControl(*m_CtlIt);
			m_CtlIt++;
		}
		m_CtlList.Clear();
	}

	Base::Destroy();
}

bool Window::RegisterControl(NWindow::Control *pControl)
{
	if (!pControl) return false;

	bool b = m_CtlList.Find(pControl);
	if (b) return true;	// Return true if already registered

	b = m_CtlList.PushBack(pControl);
	if (b) {
		pControl->m_pWind = this;
		pControl->m_clWindBkg = m_clWhite;
		pControl->m_iXOffset = 0;
		pControl->m_iYOffset = m_ciHdrHeight;
		pControl->m_rc.Offset(0, m_ciHdrHeight);
		//pControl->m_Disp = m_Disp;
		pControl->m_Disp.Destroy();
	}

	return b;
}

bool Window::DeregisterControl(NWindow::Control *pControl)
{
	if (!pControl) return false;

	bool b = m_CtlList.Remove(pControl);
	if (b) {
		pControl->m_pWind = 0;
		pControl->m_Disp.Destroy();
	}
	
	return b;
}

bool Window::Show()
{
	bool b = Select(dynamic_cast<Base *>(this));
	if (!b) return false;

	m_Disp.GetRect(m_rcDisp);

	m_rcClient = m_rcDisp;
	m_rcClient.Bottom -= m_ciHdrHeight;

	m_rcHeader = m_rcDisp;
	m_rcHeader.Bottom = m_rcHeader.Top + m_ciHdrHeight;

	m_rcClose = m_rcHeader;
	m_rcClose += -3;
	m_rcClose.Left = m_rcClose.Right - m_rcClose.Height();

	/*
	 * *** Start drawing ***
	 */
	m_Disp.Fill(m_clWhite);

	_DrawHeader();

	m_CtlIt = m_CtlList.Begin();
	while (m_CtlIt != m_CtlList.End())
	{
		(*m_CtlIt)->m_Disp.Create();
		(*m_CtlIt)->_Draw();
		m_CtlIt++;
	}

	m_bIsShown = true;
	OnShow();
	
	return true;
}

bool Window::ProcessInput(NWindow::Input &Inp)
{
	bool		b = false;

	switch (Inp.Type)
	{
	case InputType::Touched:
	case InputType::Released:
		b = _HandleTouchRelease(Inp);
		break;

	case InputType::Char:
	case InputType::CommandChar:
		b = _HandleChar(Inp);
		break;

	default:
		break;
	}

	return b;
}

bool Window::IsShown()
{
	return m_bIsShown;
}

bool Window::GetClientRectangle(Rectangle &rcRes)
{
	if (m_rcClient.IsEmpty()) {
		m_Disp.GetRect(rcRes);
		rcRes.Bottom -= m_ciHdrHeight;
	} else {
		rcRes = m_rcClient;
	}
	return true;
}

Control::Control():
m_pWind(0),
m_clBlack(RGB(0,0,0)),
m_clWhite(RGB(255,255,255))
{
	Left.Set(this, &Control::_SetLeft, &Control::_GetLeft);
	Top.Set(this, &Control::_SetTop, &Control::_GetTop);
	Width.Set(this, &Control::_SetWidth, &Control::_GetWidth);
	Height.Set(this, &Control::_SetHeight, &Control::_GetHeight);
	Visible.Set(this, &Control::_SetVisible, &Control::_GetVisible);

	m_bVisible = true;
}

Control::~Control()
{
	Destroy();
}

void Control::_SetLeft(int iLeft)
{
	m_rcUpdate = m_rc;

	int dx = iLeft - (m_rc.Left - m_iXOffset);
	m_rc.Offset(dx, 0);
	m_rcUpdate |= m_rc;
	
	_Draw();
}

void Control::_SetTop(int iTop)
{
	m_rcUpdate = m_rc;
	
	int dy = iTop - (m_rc.Top - m_iYOffset);
	m_rc.Offset(0, dy);
	m_rcUpdate |= m_rc;

	_Draw();
}

void Control::_SetWidth(int iWidth)
{
	m_rcUpdate = m_rc;
	m_rc.Right = (m_rc.Left + iWidth);
	m_rcUpdate |= m_rc;

	_Draw();
}

void Control::_SetHeight(int iHeight)
{
	m_rcUpdate = m_rc;
	m_rc.Bottom = (m_rc.Top + iHeight);
	m_rcUpdate |= m_rc;
	
	_Draw();
}

void Control::_SetVisible(bool bEnable)
{
	if (m_bVisible != bEnable) {
		m_bVisible = bEnable;

		if (bEnable) {
			_Draw();
		} else {
			m_rcUpdate = m_rc;
			m_rcUpdate += 1;
			m_Disp.Fill(m_clWindBkg, m_rcUpdate);
		}
	}
}

int Control::_GetLeft()
{
	return (m_rc.Left - m_iXOffset);
}

int Control::_GetTop()
{
	return (m_rc.Top - m_iYOffset);
}

int Control::_GetWidth()
{
	return m_rc.Width();
}

int Control::_GetHeight()
{
	return m_rc.Height();
}

bool Control::_GetVisible()
{
	return m_bVisible;
}

bool Control::Create(int iLeft, int iTop, int iWidth, int iHeight, NWindow::Window &Wind)
{
	m_rc.Set(iLeft, iTop, (iLeft + iWidth), (iTop + iHeight));
	m_bVisible = true;
	
	bool b = Wind.RegisterControl(this);
	if (!b) return false;

	b = _Create();
	if (!b) {
		Wind.DeregisterControl(this);
	}

	return b;
}

void Control::Destroy()
{
}

Label::Label()
{
	Text.Set(this, &Label::_SetText, &Label::_GetText);
	Border.Set(this, &Label::_SetBorder, &Label::_GetBorder);
	MultiLine.Set(this, &Label::_SetMultiLine, &Label::_GetMultiLine);
	WordWrap.Set(this, &Label::_SetWordWrap, &Label::_GetWordWrap);
	EndEllipsis.Set(this, &Label::_SetEndEllipsis, &Label::_GetEndEllipsis);
	Align.Set(this, &Label::_SetAlign, &Label::_GetAlign);
	TextScale.Set(this, &Label::_SetTextScale, &Label::_GetTextScale);
	LineSpace.Set(this, &Label::_SetLineSpace, &Label::_GetLineSpace);
}

Label::~Label()
{
}

bool Label::_Create()
{
	m_rcText = m_rc;
	m_rcText += -4;

	m_strText		= "Label";
	m_bBorder		= false;
	
	m_Disp.m_bEndEllipsis	= true;
	m_Disp.m_bMultiLine		= false;
	m_Disp.m_bWordWrap		= false;
	m_Disp.m_eTextAlign		= TextAlign::LeftTop;
	m_Disp.m_iTextScale		= 1;
	m_Disp.m_iLineSpace		= 2;

	m_Disp.SetTextColor(m_clBlack, m_clWhite, false);

	return true;
}

bool Label::_Draw()
{
	bool b = false;

	if (!m_bVisible) return true;

	m_Disp.Fill(m_clWindBkg, m_rcUpdate);

	if (m_bBorder) {
		b = m_Disp.DrawRect(m_rc, m_clBlack);
	}

	b = m_Disp.WriteText(m_strText.CStr(), m_rcText);

	return b;
}

bool Label::_ProcessInput(NWindow::Input &Inp)
{
	return true;
}

TextAlign::E Label::_GetAlign()
{
	return m_Disp.m_eTextAlign;
}

bool Label::_GetBorder()
{
	return m_bBorder;
}

bool Label::_GetEndEllipsis()
{
	return m_Disp.m_bEndEllipsis;
}

int Label::_GetLineSpace()
{
	return m_Disp.m_iLineSpace;
}

bool Label::_GetMultiLine()
{
	return m_Disp.m_bMultiLine;
}

int Label::_GetTextScale()
{
	return m_Disp.m_iTextScale;
}

const char * Label::_GetText()
{
	return m_strText.CStr();
}

bool Label::_GetWordWrap()
{
	return m_Disp.m_bWordWrap;
}

void Label::_SetAlign(NWindow::TextAlign::E eAlign)
{
	if (m_Disp.m_eTextAlign != eAlign) {
		m_Disp.m_eTextAlign = eAlign;

		m_rcUpdate = m_rcText;
		_Draw();
	}
}

void Label::_SetBorder(bool bEnable)
{
	if (m_bBorder != bEnable) {
		m_bBorder = bEnable;

		m_rcUpdate = m_rcText;
		_Draw();
	}
}

void Label::_SetEndEllipsis(bool bEnable)
{
	if (m_Disp.m_bMultiLine)
		return;

	if (m_Disp.m_bEndEllipsis != bEnable) {
		m_Disp.m_bEndEllipsis = bEnable;

		m_rcUpdate = m_rcText;
		_Draw();
	}
}

void Label::_SetLineSpace(int iLineSpace)
{
	if (m_Disp.m_iLineSpace != iLineSpace) {
		m_Disp.m_iLineSpace = (iLineSpace <= 0) ? 0 : iLineSpace;

		m_rcUpdate = m_rcText;
		_Draw();
	}
}

void Label::_SetMultiLine(bool bEnable)
{
	if (m_Disp.m_bMultiLine != bEnable) {
		if (m_Disp.m_bEndEllipsis) m_Disp.m_bEndEllipsis = false;
		m_Disp.m_bMultiLine = bEnable;

		m_rcUpdate = m_rcText;
		_Draw();
	}
}

void Label::_SetText(const char *szText)
{
	m_strText = szText;
	m_rcUpdate = m_rcText;
	_Draw();
}

void Label::_SetTextScale(int iScale)
{
	if (m_Disp.m_iTextScale != iScale) {
		m_Disp.m_iTextScale = (iScale <= 1) ? 1 : iScale;

		m_rcUpdate = m_rcText;
		_Draw();
	}
}

void Label::_SetWordWrap(bool bEnable)
{
	if (!m_Disp.m_bMultiLine)
		return;

	if (m_Disp.m_bWordWrap != bEnable) {
		m_Disp.m_bWordWrap = bEnable;

		m_rcUpdate = m_rcText;
		_Draw();
	}
}

Button::Button()
{
	Text.Set(this, &Button::_SetText, &Button::_GetText);
	TextScale.Set(this, &Button::_SetTextScale, &Button::_GetTextScale);
}

Button::~Button()
{
}

bool Button::_Draw()
{
	bool b = false;

	if (!m_bVisible) return true;

	m_Disp.Fill(m_clWindBkg, m_rcUpdate);

	if (m_bTouched) {
		m_Disp.SetTextColor(m_clBlack, m_clWhite, true);
	} else {
		m_Disp.Fill(m_clBkgCol, m_rc);
	}
	
	b = m_Disp.WriteText(m_strText.CStr(), m_rc);
	b = m_Disp.DrawRect(m_rc, m_clBlack);

	return b;
}

bool Button::_ProcessInput(NWindow::Input &Inp)
{
	bool b = false;

	switch (Inp.Type)
	{
	case InputType::Touched:
		m_bTouched = true;
		b = true;
		break;

	case InputType::Released:
		if (m_bTouched) {
			OnTouched();
			b = true;
		}
		m_bTouched = false;
		break;

	default:
		break;
	}

	if (b) {
		m_rcUpdate = m_rc;
		_Draw();
	}

	return b;
}

bool Button::_Create()
{
	m_strText		= "Button";
	m_clBkgCol		= RGB(225,225,225);
	m_bTouched		= false;

	m_Disp.m_bEndEllipsis	= false;
	m_Disp.m_bMultiLine		= false;
	m_Disp.m_bWordWrap		= false;
	m_Disp.m_eTextAlign		= TextAlign::Center;
	m_Disp.m_iLineSpace		= 0;
	m_Disp.m_iTextScale		= 1;

	m_Disp.SetTextColor(m_clBlack, m_clBkgCol, false);

	return true;
}

void Button::_SetText(const char *szText)
{
	m_strText = szText;
	m_rcUpdate = m_rc;
	_Draw();
}

void Button::_SetTextScale(int iScale)
{
	if (m_Disp.m_iTextScale != iScale) {
		m_Disp.m_iTextScale = (iScale < 1) ? 1 : iScale;

		m_rcUpdate = m_rc;
		_Draw();
	}
}

const char * Button::_GetText()
{
	return m_strText.CStr();
}

int Button::_GetTextScale()
{
	return m_Disp.m_iTextScale;
}

};	// End of namespace NWindow
