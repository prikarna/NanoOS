/*
 * File    : NSvcControl.h
 * Remark  : NSvcControl class definition.
 *           To control NPortSvc service.
 *
 */

#pragma once
#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"

#include "..\NTerminal\DkObject.h"

class NSvcControl
{
public:
	NSvcControl();
	~NSvcControl();

	enum SvcStatus {
		Unknown,
		Stopped,
		StartPending,
		StopPending,
		Running,
		ContinuePending,
		PuasePending,
		Paused
	};

	DWORD GetError();

	bool Install(HWND hWnd);
	bool Uninstall();
	bool Start();
	bool Stop();
	bool Pause();
	bool Resume();
	bool IsInstalled(bool & bResult);
	bool GetStatus(NSvcControl::SvcStatus & Status);

	Event<const TCHAR *>	OnError;

private:
	SC_HANDLE		m_hSvcMgr;
	SC_HANDLE		m_hSvc;

	DWORD			m_dwError;
	const TCHAR *	m_szDefName;
	const TCHAR *	m_szDefDispName;
	const TCHAR *	m_szDefDesc;
	TCHAR			m_szFileName[512];
	OPENFILENAME	m_ofn;
	TCHAR			m_szErrBuf[1024];

	bool _OpenSCM(DWORD dwDesiredAccess);
	bool _Open(const TCHAR * szServiceName, DWORD dwDesiredAccess);
	void _CloseSCM();
	void _Close();
	bool _Control(DWORD dwControl);
	bool _QueryStatus(LPSERVICE_STATUS pStatus);

	void _HandleError(DWORD dwError);
};