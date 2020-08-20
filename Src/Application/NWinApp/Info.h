#pragma once

#include "..\NWindow.h"

using namespace NWindow;

class Info: public EventReceiver
{
private:
	Input	m_Input;

	Window	m_Wind;
	Label	m_lbOSName;
	Label	m_lbOSVer;
	Label	m_lbChipset;
	Label	m_lbSpeed;
	Label	m_lbMaxThd;
	Label	m_lbMem;
	Button	m_btnShow;
	Label	m_lbLogo;

	void _HandleButtonTouched();

public:
	Info();
	~Info();

	bool Show();
};