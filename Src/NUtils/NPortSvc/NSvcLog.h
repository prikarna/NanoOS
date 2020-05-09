/*
 * File    : NSvcLog.h"
 * Remark  : Service event log class.
 *
 */

#pragma once

#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"

#include "NSvcName.h"

class NSvcLog
{
public:
	NSvcLog();
	~NSvcLog();

	enum Type {
		Typ_Error,
		Typ_Warning,
		Typ_Info
	};

	bool Register(const TCHAR *szSvcBinPath);
	bool Unregister();
	DWORD GetError();
	bool Report(NSvcLog::Type type, const TCHAR * szMessage);

private:
	const TCHAR *	m_cszLogName;
	const TCHAR *	m_cszSvcSource;
	const TCHAR *	m_cszSvcName;
	const TCHAR *	m_cszLogFileName;
	DWORD			m_dwError;
};
