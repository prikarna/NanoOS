/*
 * File    : NWindow.h
 * Remark  : A simple 'windowing system' for GUI application.
 *           Provide basic infrastructure to create GUI application, for now only works on VDisplay
 *           which available in NTerminal.
 *
 */

#pragma once

#include "NDisplay.h"

namespace NWindow
{

class EventReceiver
{
public:
	EventReceiver() {}
	~EventReceiver() {}
};

template <typename ArgType=int>
class EventHandler
{
public:
	EventReceiver *			m_pReceiver;
	void (EventReceiver:: *	m_pHandler)(ArgType);
	void (EventReceiver:: * m_pHandlerNoArg)();

	EventHandler<ArgType>()
	{
		m_pReceiver		= 0;
		m_pHandler		= 0;
		m_pHandlerNoArg	= 0;
	}

	~EventHandler<ArgType>() {}
};

/*
	Event template class.
	To support event driven programming model in this 'windowing system'
*/
template <typename Owner, typename ArgType=int>
class Event
{
	EventReceiver			* m_pReceiver;
	void (EventReceiver::	* m_pRecvHandler)(ArgType);
	void (EventReceiver::	* m_pRecvHandlerNoArg)();

	void (*					m_pHandler)(ArgType);
	void (*					m_pHandlerNoArg)();

	void Fire(ArgType Arg)
	{
		if ((m_pReceiver) && (m_pRecvHandler))
		{
			(m_pReceiver->*m_pRecvHandler)(Arg);
		}
		else
		{
			if (m_pHandler)
				(* m_pHandler)(Arg);
		}
	}

	void Fire()
	{
		if ((m_pReceiver) && (m_pRecvHandlerNoArg))
		{
			(m_pReceiver->*m_pRecvHandlerNoArg)();
		}
		else
		{
			if (m_pHandlerNoArg)
				(* m_pHandlerNoArg)();
		}
	}

public:
	Event<Owner, ArgType>()
	{
		m_pReceiver			= 0;
		m_pRecvHandler		= 0;
		m_pRecvHandlerNoArg	= 0;
		m_pHandler			= 0;
		m_pHandlerNoArg		= 0;
	}

	~Event<Owner, ArgType>(){}

	Event<Owner, ArgType> operator =(void (* Handler)(ArgType))
	{
		m_pHandler = Handler;
		return *this;
	}

	Event<Owner, ArgType> operator =(EventHandler<ArgType> Handler)
	{
		if (Handler.m_pReceiver) {
			m_pReceiver	= Handler.m_pReceiver;
			if (Handler.m_pHandler) {
				m_pRecvHandler		= Handler.m_pHandler;
			} else {
				m_pRecvHandlerNoArg	= Handler.m_pHandlerNoArg;
			}
		}
		return *this;
	}

	Event<Owner, ArgType> operator =(void (* HandlerNoArg)())
	{
		m_pHandlerNoArg = HandlerNoArg;
		return *this;
	}

	//friend typename Owner;

	/*
	 Since GCC doesn't support friend typename ..., we must provide
	 access to 'dispatch' the event publicly and this is bad! 
	 This will allow non owner of the event can generate the event.
	 */
	Event<Owner, ArgType> operator ()(ArgType Arg)
	{
		Fire(Arg);
		return *this;
	}

	Event<Owner, ArgType> operator ()()
	{
		Fire();
		return *this;
	}
};

/*
	Property template class.
	Provide 'active properties' for a window and control class.
*/
template <typename Owner, typename VarType>
class Property
{
	Owner *				m_pOwner;
	void (Owner::*		m_Setter)(VarType);
	VarType (Owner::*	m_Getter)();

public:
	Property<Owner, VarType>()
	{
		m_pOwner	= 0;
		m_Setter	= 0;
		m_Getter	= 0;
	}

	~Property<Owner, VarType>() {}

	void Set(Owner *pOwner, void (Owner::* Setter)(VarType), VarType (Owner::* Getter)())
	{
		if (m_pOwner == 0)
			m_pOwner = pOwner;
		if (m_Setter == 0)
			m_Setter = Setter;
		if (m_Getter == 0)
			m_Getter = Getter;
	}

	Property<Owner, VarType> operator =(VarType Var)
	{
		if ((m_pOwner) && (m_Setter))
		{
			(m_pOwner->*m_Setter)(Var);
		}
		return *this;
	}

	Property<Owner, VarType> operator =(Property<Owner, VarType> RightProp)
	{
		VarType	Var;

		NanoStd::MemSet<VarType>(&Var, 1, 0);

		if ((RightProp.m_pOwner) &&
			(RightProp.m_Getter) &&
			(m_pOwner) &&
			(m_Setter))
		{
			Var = ((RightProp.m_pOwner)->*(RightProp.m_Getter))();
			(m_pOwner->*m_Setter)(Var);
		}

		return *this;
	}

	operator VarType() const
	{
		VarType Var;

		NanoStd::MemSet<VarType>(&Var, 1, 0);

		if ((m_pOwner) && (m_Getter))
		{
			Var = (m_pOwner->*m_Getter)();
		}

		return Var;
	}
};

/* 
	Int template class, 
	can only be instantiate with built-in type with known fixed size such as int, 
	if not undefined behaviour may occured.
 */
template <typename BuiltinType>
class Int
{
	BuiltinType					IntVar;
	NanoStd::String<char, 10>	Buffer;

public:
	typedef NanoStd::String<char, 10>	IntString;

	Int<BuiltinType>() { IntVar = 0; }
	~Int<BuiltinType>() {}

	Int<BuiltinType>(BuiltinType InitVal) { IntVar = InitVal; }

	Int<BuiltinType> operator =(BuiltinType Value)
	{
		IntVar = Value;
		return *this;
	}

	Int<BuiltinType> operator +=(BuiltinType Value)
	{
		IntVar += Value;
		return *this;
	}

	Int<BuiltinType> operator -=(BuiltinType Value)
	{
		IntVar -= Value;
		return *this;
	}

	Int<BuiltinType> operator ++(int i)	// As postfix of increment
	{
		Int<BuiltinType>	Ret = *this;
		IntVar++;
		return Ret;
	}

	Int<BuiltinType> operator --(int i)	// As postfix of decrement
	{
		Int<BuiltinType>	Ret = *this;
		IntVar--;
		return Ret;
	}

	Int<BuiltinType> operator ++()			// As prefix
	{
		IntVar++;
		return *this;
	}

	Int<BuiltinType> operator --()			// As prefix
	{
		IntVar--;
		return *this;
	}

	bool operator ==(Int<BuiltinType> RightInt)
	{
		return (IntVar == RightInt.IntVar) ? true : false;
	}

	bool operator !=(Int<BuiltinType> RightInt)
	{
		return (IntVar != RightInt.IntVar) ? true : false;
	}

	bool operator <(Int<BuiltinType> RightInt)
	{
		return (IntVar < RightInt.IntVar) ? true : false;
	}

	bool operator >(Int<BuiltinType> RightInt)
	{
		return (IntVar > RightInt.IntVar) ? true : false;
	}

	bool operator <=(Int<BuiltinType> RightInt)
	{
		return (IntVar <= RightInt.IntVar) ? true : false;
	}

	bool operator >=(Int<BuiltinType> RightInt)
	{
		return (IntVar >= RightInt.IntVar) ? true : false;
	}

	operator BuiltinType() const
	{
		return IntVar;
	}

	bool operator ==(BuiltinType Value)
	{
		return (IntVar == Value) ? true : false;
	}

	bool operator !=(BuiltinType Value)
	{
		return (IntVar != Value) ? true : false;
	}

	bool operator <(BuiltinType Value)
	{
		return (IntVar < Value) ? true : false;
	}

	bool operator >(BuiltinType Value)
	{
		return (IntVar > Value) ? true : false;
	}

	bool operator <=(BuiltinType Value)
	{
		return (IntVar <= Value) ? true : false;
	}

	bool operator >=(BuiltinType Value)
	{
		return (IntVar >= Value) ? true : false;
	}

	const char * ToText()
	{
		Buffer.Clear();
		StreamPrintf(reinterpret_cast<UINT8_PTR_T>(Buffer.GetBuffer()), Buffer.MaxBuffer(), "%d", IntVar);
		return Buffer.CStr();
	}

	IntString ToString()
	{
		Buffer.Clear();
		StreamPrintf(reinterpret_cast<UINT8_PTR_T>(Buffer.GetBuffer()), Buffer.MaxBuffer(), "%d", IntVar);
		return Buffer;
	}
};

struct Error
{
	typedef NanoStd::String<char, 16>	TString;

	unsigned int	ErrCode;
	TString			Message;

	bool operator ==(Error RightErr)
	{
		return (ErrCode == RightErr.ErrCode) ? true : false;
	}

	bool operator !=(Error RightErr)
	{
		return (ErrCode != RightErr.ErrCode) ? true : false;
	}

	Error operator =(const char * szText)
	{
		Message = szText;
		return *this;
	}

	Error operator =(unsigned int RightErrCode)
	{
		ErrCode = RightErrCode;
		return *this;
	}

	unsigned int Code()
	{
		return ErrCode;
	}

	void Set(unsigned int Code, const char * szMessage)
	{
		ErrCode = Code;
		Message = szMessage;
	}
};

struct InputType
{
	enum E {
		None,
		Touched,
		Released,
		Char,
		CommandChar
	};
};

struct CommandChar
{
	enum E {
		None	= 0x00,
		Back	= 0x08,
		Return	= 0x0D,
		Capital	= 0x14,
		Escape	= 0x1B,
		Left	= 0x25,
		Up		= 0x26,
		Right	= 0x27,
		Down	= 0x28,
		Insert	= 0x2D,
		Delete	= 0x2E
	};
};

struct Input
{
	InputType::E	Type;
	unsigned int	Param1;
	//unsigned int	Param2;

	void ToPoint(Point & pt)
	{
		pt.x = (Param1 & 0xFFFF);
		pt.y = ((Param1 >> 16) & 0xFFFF);
	}

	void FromPoint(Point & pt)
	{
		Param1 = ((pt.x & 0xFFFF) | ((((unsigned int)(pt.y)) & 0xFFFF) << 16));
	}

	void FromPoint(int iX, int iY)
	{
		Param1 = ((iX & 0xFFFF) | ((((unsigned int)(iY)) & 0xFFFF) << 16));
	}

	char ToChar()
	{
		return static_cast<char>(Param1);
	}

	CommandChar::E CmdChar()
	{
		CommandChar::E cmd;
		switch (static_cast<CommandChar::E>(Param1))
		{
		case CommandChar::Back:
		case CommandChar::Capital:
		case CommandChar::Delete:
		case CommandChar::Down:
		case CommandChar::Escape:
		case CommandChar::Insert:
		case CommandChar::Left:
		case CommandChar::Return:
		case CommandChar::Right:
		case CommandChar::Up:
			cmd = static_cast<CommandChar::E>(Param1);
			break;

		default:
			cmd = CommandChar::None;
			break;
		}

		return cmd;
	}

	void FromChar(char ch)
	{
		Param1 = static_cast<unsigned int>(ch);
	}
};

/* Window base class, all window class should derive from this class */
class Base
{
private:
	static Base *					m_pCurWind;
	static NanoStd::List<Base *, 4>	m_List;
	static bool						m_bStopDispInput;
	static unsigned int				m_uiInTID;
	
	static int _DispInputLoop(void *pDat);

protected:
	COLORREF					m_clBlack;
	COLORREF					m_clWhite;
	COLORREF					m_clGr64;
	COLORREF					m_clGr128;
	COLORREF					m_clGr192;
	Display						m_Disp;
	NanoStd::Queue<Input, 4>	m_InQue;
	Rectangle					m_rcDisp;

	bool Select(Base *pWind);
	bool IsCurrent();

public:
	Base();
	~Base();

	bool Create();
	virtual bool Show() = 0;
	void Destroy();
	virtual bool ProcessInput(Input & Inp) = 0;

	static bool PutInput(Input & Inp);
	bool GetInput(Input & Inp);

	static bool StartDisplayInput();
	static bool StopDisplayInput();

	Event<Base, int>			OnCreate;
	Event<Base, int>			OnShow;
	Event<Base, int>			OnDestroy;
	Event<Base, const Point &>	OnTouched;
	Event<Base, const Point &>	OnReleased;
};

struct Icon
{
	Int<unsigned int>			m_Id;
	NanoStd::String<char, 16>	m_szName;
	Rectangle					m_rc;
	Rectangle					m_rcImg;
	Rectangle					m_rcText;
	bool						m_bEnable;

	Icon()
	{
		m_bEnable = false;
	}

	~Icon() {}

	bool operator ==(Icon RightIcon)
	{
		if (m_Id == RightIcon.m_Id)
			return true;
		return false;
	}
};

/* A Window but doesn't have header and can't contain Control class */
class Screen: public Base
{
private:
	struct IconColumns
	{
		NanoStd::Array<Icon, 3>			m_Columns;
	};
	struct IconRows
	{
		NanoStd::Array<IconColumns, 2>	m_Rows;
	};

	IconRows		m_Grid;
	bool			m_bEnaIconContainer;
	bool			m_bIconTouched;
	int				m_ciIconWidth;
	int				m_ciIconHeight;
	Icon *			m_pSelIcon;

	bool _ShowIcons();
	bool _ShowScreen();
	bool _DrawIcon(Icon * pCurIcon);
	bool _DefInputHandler(Input & Inp);

	void _SetEnaIconContainer(bool bEnable);
	bool _GetEnaIconContainer();

	void _TestDisplay();

public:
	Screen();
	~Screen();

	bool Show();
	bool ProcessInput(Input & Inp);

	Event<Screen, unsigned int>		OnIconTouched;

	Property<Screen, bool>	IconContainer;
};

class Control;

/* Window class, can contain control class */
class Window: public Base
{
	Rectangle		m_rcClose;
	Rectangle		m_rcIcon;
	Rectangle		m_rcHeader;
	Rectangle		m_rcClient;
	const int		m_ciHdrHeight;
	bool			m_bIconTouched;
	bool			m_bCloseTouched;
	bool			m_bIsShown;

	NanoStd::String<char, 16>				m_strTitle;
	NanoStd::List<Control *, 8>				m_CtlList;
	NanoStd::List<Control *, 8>::Iterator	m_CtlIt;

	bool _DrawHeader();
	bool _DrawCloseButon(bool bIsTouched);

	void _SetText(const char *szText);
	const char * _GetText();

	bool _HandleTouchRelease(Input & Inp);
	bool _DefaultTouchRelease(Input & Inp);
	bool _HandleChar(Input & Inp);

public:
	Window(void);
	~Window(void);

	bool Show();
	bool ProcessInput(Input & Inp);
	bool IsShown();
	void Destroy();
	bool GetClientRectangle(Rectangle &rcRes);

	Event<Window, bool &>	OnClose;
	Event<Window, int>		OnIconTouched;
	Event<Window, Point &>	OnHeaderTouched;

	Property<Window, const char *>	Text;

	bool RegisterControl(Control *pControl);
	bool DeregisterControl(Control *pControl);
};

/* Control base class, all Control such as Button class should derive from this */
class Control
{
private:

protected:
	Rectangle	m_rc;
	Rectangle	m_rcUpdate;
	Window *	m_pWind;
	Display		m_Disp;
	COLORREF	m_clBlack;
	COLORREF	m_clWhite;
	COLORREF	m_clWindBkg;
	int			m_iXOffset;
	int			m_iYOffset;
	bool		m_bFocused;
	bool		m_bVisible;

	virtual bool _Draw() = 0;
	virtual bool _ProcessInput(Input &Inp) = 0;
	virtual bool _Create() { return true; };

	void _SetLeft(int iLeft);
	void _SetTop(int iTop);
	void _SetWidth(int iWidth);
	void _SetHeight(int iHeight);
	void _SetVisible(bool bEnable);
	int _GetLeft();
	int _GetTop();
	int _GetWidth();
	int _GetHeight();
	bool _GetVisible();

public:
	Control(void);
	~Control(void);

	bool Create(int iLeft, int iTop, int iWidth, int iHeight, Window &Wind);
	void Destroy();

	Property<Control, int>	Left;
	Property<Control, int>	Top;
	Property<Control, int>	Width;
	Property<Control, int>	Height;
	Property<Control, bool>	Visible;

	friend class Window;
};

/* Label class, to show text in NWindow application */
class Label: public Control
{
private:
	typedef NanoStd::String<char, 128>	TString;

	TString		m_strText;
	Rectangle	m_rcText;
	bool		m_bBorder;

	bool _Draw();
	bool _ProcessInput(Input & Inp);
	bool _Create();

	void _SetBorder(bool bEnable);
	void _SetMultiLine(bool bEnable);
	void _SetWordWrap(bool bEnable);
	void _SetEndEllipsis(bool bEnable);
	void _SetAlign(TextAlign::E eAlign);
	void _SetTextScale(int iScale);
	void _SetLineSpace(int iLineSpace);
	void _SetText(const char *szText);

	bool _GetBorder();
	bool _GetMultiLine();
	bool _GetWordWrap();
	bool _GetEndEllipsis();
	TextAlign::E _GetAlign();
	int _GetTextScale();
	int _GetLineSpace();
	const char * _GetText();

public:
	Label();
	~Label();

	Property<Label, bool>		Border;
	Property<Label, bool>		MultiLine;
	Property<Label, bool>		WordWrap;
	Property<Label, bool>		EndEllipsis;
	Property<Label, TextAlign::E>	Align;
	Property<Label, int>		TextScale;
	Property<Label, int>		LineSpace;
	Property<Label, const char *>	Text;
};

/* Button class, can interract with the user in NWindow application by touch it */
class Button: public Control
{
private:
	typedef NanoStd::String<char, 64>	TString;

	TString			m_strText;
	bool			m_bTouched;
	COLORREF		m_clBkgCol;

	bool _Draw();
	bool _ProcessInput(Input & Inp);
	bool _Create();

	void _SetText(const char *szText);
	void _SetTextScale(int iScale);

	const char * _GetText();
	int _GetTextScale();

public:
	Button();
	~Button();

	Property<Button, const char *>	Text;
	Property<Button, int>			TextScale;

	Event<Button, int>				OnTouched;
};

};	// End of namespace NWindow
