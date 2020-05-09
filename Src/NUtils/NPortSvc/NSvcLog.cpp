/*
 * File    : NSvcLog.h"
 * Remark  : NSvcLog class implementation.
 *
 */

#include "NSvcLog.h"
#include "NSvcName.h"
#include "..\NTerminal\Debug.h"
#include "NSvcLogMsg.h"

NSvcLog::NSvcLog():
m_cszLogName(_T("Application")),
m_cszSvcSource(_T("NPortSvc")),
m_cszLogFileName(_T("NSvcLogMsg.dll")),
m_cszSvcName(NSVC_DEF_NAME)
{
}

NSvcLog::~NSvcLog()
{
}

bool NSvcLog::Register(const TCHAR *szSvcBinPath)
{
	TCHAR		szLogKey[256] = {0};
	TCHAR		szLogFileName[1024] = {0};
	HRESULT		hr = S_OK;
	bool		bRes = false;
	LSTATUS		lStat = 0;
	HKEY		hk = NULL;
	DWORD		dwDisp = 0, dwData = 0, dwCatNumb = 0;

	do {
		hr = StringCbPrintf(
				szLogFileName,
				sizeof(szLogFileName),
				_T("%s\\%s"),
				szSvcBinPath,
				m_cszLogFileName
				);
		if (hr != S_OK) {
			m_dwError = HRESULT_CODE(hr);
			break;
		}

		hr = StringCbPrintf(
				szLogKey, 
				sizeof(szLogKey), 
				_T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\%s\\%s"),
				m_cszLogName, 
				m_cszSvcSource
				);
		if (hr != S_OK) {
			m_dwError = HRESULT_CODE(hr);
			break;
		}
		
		lStat = RegCreateKeyEx(
					HKEY_LOCAL_MACHINE,
					szLogKey,
					0,
					NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_WRITE,
					NULL,
					&hk,
					&dwDisp
					);
		if (lStat != ERROR_SUCCESS) {
			m_dwError = GetLastError();
			DBG_PRINTF(_T("%s: Error create event log key! (%d)\r\n"), FUNCT_NAME_STR, m_dwError);
			break;
		}

		lStat = RegSetValueEx(
					hk,
					_T("EventMessageFile"),
					0,
					REG_EXPAND_SZ,
					(const BYTE *) szLogFileName,
					(DWORD) ((lstrlen(szLogFileName) + 1) * sizeof(TCHAR))
					);
		if (lStat != ERROR_SUCCESS) {
			m_dwError = GetLastError();
			DBG_PRINTF(_T("%s: Error set EventMessageFile value! (%d)\r\n"), FUNCT_NAME_STR, m_dwError);
			break;
		}

		dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_INFORMATION_TYPE | EVENTLOG_WARNING_TYPE;
		lStat = RegSetValueEx(
					hk,
					_T("TypesSupported"),
					0,
					REG_DWORD,
					(const BYTE *) &dwData,
					sizeof(DWORD)
					);
		if (lStat != ERROR_SUCCESS) {
			m_dwError = GetLastError();
			DBG_PRINTF(_T("%s: Error set TypesSupported value! (%d)\r\n"), FUNCT_NAME_STR, m_dwError);
			break;
		}

		lStat = RegSetValueEx(
					hk,
					_T("CategoryMessageFile"),
					0,
					REG_EXPAND_SZ,
					(const BYTE *) szLogFileName,
					(DWORD) ((lstrlen(szLogFileName) + 1) * sizeof(DWORD))
					);
		if (lStat != ERROR_SUCCESS) {
			m_dwError = GetLastError();
			DBG_PRINTF(_T("%s: Error set CategoryMessageFile value! (%d)\r\n"), FUNCT_NAME_STR, m_dwError);
			break;
		}

		dwCatNumb = 1;
		lStat = RegSetValueEx(
					hk,
					_T("CategoryCount"),
					0,
					REG_DWORD,
					(const BYTE *) &dwCatNumb,
					sizeof(DWORD)
					);
		if (lStat != ERROR_SUCCESS) {
			m_dwError = GetLastError();
			DBG_PRINTF(_T("%s: Error set CategoryCount value! (%d)\r\n"), FUNCT_NAME_STR, m_dwError);
			break;
		}

		bRes = true;
	} while (FALSE);

	if (hk) RegCloseKey(hk);
	
	return bRes;
}

bool NSvcLog::Unregister()
{
	TCHAR		szEvtLogPath[256] = {0};
	HRESULT		hr = S_OK;
	bool		bRes = false;
	LSTATUS		lStat = 0;
	HKEY		hk = NULL;

	do {
		hr = StringCbPrintf(
				szEvtLogPath, 
				sizeof(szEvtLogPath), 
				_T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\%s"),
				m_cszLogName 
				);
		if (hr != S_OK) {
			m_dwError = HRESULT_CODE(hr);
			break;
		}
		
		lStat = RegOpenKeyEx(
					HKEY_LOCAL_MACHINE,
					szEvtLogPath,
					0,
					KEY_WRITE,
					&hk
					);
		if (lStat != ERROR_SUCCESS) {
			m_dwError = GetLastError();
			DBG_PRINTF(_T("%s: Error open event log key! (%d)\r\n"), FUNCT_NAME_STR, m_dwError);
			break;
		}

		lStat = RegDeleteKey(
					hk,
					m_cszSvcSource
					);
		if (lStat != ERROR_SUCCESS) {
			m_dwError = GetLastError();
			DBG_PRINTF(_T("%s: Error delete event source key! (%d)\r\n"), FUNCT_NAME_STR, m_dwError);
			break;
		}

		bRes = true;
	} while (FALSE);

	if (hk) RegCloseKey(hk);
	
	return bRes;
}

DWORD NSvcLog::GetError()
{
	return m_dwError;
}

bool NSvcLog::Report(NSvcLog::Type type, const TCHAR * szMessage)
{
	WORD	wEvtType = 0;
	DWORD	dwEvtId = 0;
	HANDLE	hEvt = NULL;
	LPCTSTR pStrArr[2] = {0};

	if (!szMessage) {
		m_dwError = ERROR_INVALID_PARAMETER;
		return false;
	}

	pStrArr[0] = m_cszSvcName;
	pStrArr[1] = szMessage;

	switch (type)
	{
	case NSvcLog::Typ_Error:
		wEvtType = EVENTLOG_ERROR_TYPE;
		dwEvtId = NSVC_ERROR;
		break;

	case NSvcLog::Typ_Warning:
		wEvtType = EVENTLOG_WARNING_TYPE;
		dwEvtId = NSVC_WARNING;
		break;

	case NSvcLog::Typ_Info:
		wEvtType = EVENTLOG_INFORMATION_TYPE;
		dwEvtId = NSVC_INFO;
		break;

	default:
		return false;
	}

	hEvt = RegisterEventSource(NULL, m_cszSvcSource);
	if (!hEvt) {
		m_dwError = GetLastError();
		return false;
	}

	BOOL fRes = ReportEvent(
					hEvt,
					wEvtType,
					0,
					dwEvtId,
					NULL,
					2,
					0,
					pStrArr,
					NULL
					);
	if (!fRes) {
		m_dwError = GetLastError();
	}

	DeregisterEventSource(hEvt);

	return ((fRes) ? true : false);
}