/*
 * File    : NSvcControl.cpp
 * Remark  : NSvcControl class implementation.
 *
 */

#include "NSvcControl.h"
#include "..\NTerminal\Debug.h"
#include "NSvcName.h"

NSvcControl::NSvcControl():
	m_hSvcMgr(NULL),
	m_hSvc(NULL),
	m_dwError(0)
{
	m_szDefName			= NSVC_DEF_NAME;
	m_szDefDispName		= NSVC_DEF_DISP_NAME;
	m_szDefDesc			= NSVC_DEF_DESC;

	RtlZeroMemory(&m_ofn, sizeof(OPENFILENAME));
	m_ofn.lStructSize	= sizeof(OPENFILENAME);
	m_ofn.lpstrFilter	= _T("NPortSvc.exe\0\0");
	m_ofn.lpstrFile		= &m_szFileName[0];
	m_ofn.nMaxFile		= sizeof(m_szFileName)/sizeof(TCHAR);
	m_ofn.lpstrTitle	= _T("Find NPortSvc file");
	m_ofn.Flags			= OFN_FILEMUSTEXIST |
							OFN_PATHMUSTEXIST |
							OFN_NONETWORKBUTTON |
							OFN_READONLY;
	m_ofn.FlagsEx		= OFN_EX_NOPLACESBAR;
}

NSvcControl::~NSvcControl()
{
}

bool NSvcControl::_OpenSCM(DWORD dwDesiredAccess)
{
	if (m_hSvcMgr) return true;

	m_hSvcMgr = OpenSCManager(
					NULL, 
					SERVICES_ACTIVE_DATABASE, 
					dwDesiredAccess
					);
	if (!m_hSvcMgr) {
		m_dwError = GetLastError();
		_HandleError(m_dwError);
	}

	return ((m_hSvcMgr) ? true : false);
}

bool NSvcControl::_Open(const TCHAR *szServiceName, DWORD dwDesiredAccess)
{
	if (m_hSvc) return true;

	m_hSvc = OpenService(m_hSvcMgr, szServiceName, dwDesiredAccess);
	if (!m_hSvc) {
		m_dwError = GetLastError();
		_HandleError(m_dwError);
	}

	return ((m_hSvc) ? true : false);
}

void NSvcControl::_CloseSCM()
{
	if (m_hSvcMgr) {
		CloseServiceHandle(m_hSvcMgr);
		m_hSvcMgr = NULL;
	}
}

void NSvcControl::_Close()
{
	if (m_hSvc) {
		CloseServiceHandle(m_hSvc);
		m_hSvc = NULL;
	}
}

bool NSvcControl::_Control(DWORD dwControl)
{
	BOOL			fRes = FALSE;
	SERVICE_STATUS	SvcStat = {0};

	fRes = ControlService(m_hSvc, dwControl, &SvcStat);
	if (!fRes) {
		m_dwError = GetLastError();
		_HandleError(m_dwError);
	}

	return ((fRes) ? true : false);
}

bool NSvcControl::_QueryStatus(LPSERVICE_STATUS pStatus)
{
	if (!pStatus) return false;

	BOOL	fRes = FALSE;
	fRes = QueryServiceStatus(m_hSvc, pStatus);
	if (!fRes) {
		m_dwError = GetLastError();
		_HandleError(m_dwError);
	}

	return ((fRes) ? true : false);
}

bool NSvcControl::_GetFilePathFromFullPath(const TCHAR *szFullPathFileName, TCHAR *szBuffer, int iBufferByteSize)
{
	bool bRes = false;

	if ((szBuffer == NULL) || (iBufferByteSize <= 0)) return false;

	int	iFullPathSize = lstrlen(szFullPathFileName) * sizeof(TCHAR);
	if (iFullPathSize <= 0) return false;

	if (iFullPathSize > iBufferByteSize) return false;

	RtlZeroMemory(szBuffer, iBufferByteSize);
	HRESULT hr = StringCbCopy(szBuffer, iBufferByteSize, szFullPathFileName);
	if (hr != S_OK) return false;

	TCHAR *pChar = &(szBuffer[iFullPathSize - 1]);
	while (*pChar != _T('\\')) {
		*pChar = 0;
		pChar--;
	}
	*pChar = 0;

	return true;
}

DWORD NSvcControl::GetError()
{
	return m_dwError;
}

bool NSvcControl::Install(HWND hWnd)
{
	bool	bRes = false;
	BOOL	fRes = FALSE;

	do {
		RtlZeroMemory(m_szFileName, sizeof(m_szFileName));
		m_ofn.hwndOwner = hWnd;
		fRes = GetOpenFileName(&m_ofn);
		if (!fRes) {
			m_dwError = CommDlgExtendedError();
			break;
		}

		bRes = _OpenSCM(SC_MANAGER_CREATE_SERVICE);
		if (!bRes) break;

		m_hSvc = CreateService(
					m_hSvcMgr,
					m_szDefName,
					m_szDefDispName,
					GENERIC_WRITE,
					SERVICE_WIN32_OWN_PROCESS,
					SERVICE_AUTO_START,
					SERVICE_ERROR_NORMAL,
					m_szFileName,
					NULL,
					NULL,
					NULL,
					NULL,	// use LocalSystem account
					NULL
					);
		if (!m_hSvc) {
			m_dwError = GetLastError();
			_HandleError(m_dwError);
			bRes = false;
			break;
		}

		SERVICE_DESCRIPTION	sd;

		sd.lpDescription = const_cast<LPTSTR>(m_szDefDesc);
		fRes = ChangeServiceConfig2(m_hSvc, SERVICE_CONFIG_DESCRIPTION, (LPVOID) &sd);
		if (!fRes) {
			m_dwError = GetLastError();
			_HandleError(m_dwError);
			break;
		}

		bRes = true;

	} while (FALSE);

	_CloseSCM();
	_Close();

	return bRes;
}

bool NSvcControl::Install(HWND hWnd, bool bIncludeLog)
{
	bool	bRes = false;
	BOOL	fRes = FALSE;
	TCHAR	szLogPath[1024];

	do {
		RtlZeroMemory(m_szFileName, sizeof(m_szFileName));
		m_ofn.hwndOwner = hWnd;
		fRes = GetOpenFileName(&m_ofn);
		if (!fRes) {
			m_dwError = CommDlgExtendedError();
			break;
		}

		if (bIncludeLog) 
		{
			bRes = _GetFilePathFromFullPath(m_szFileName, szLogPath, sizeof(szLogPath));
			if (!bRes) break;

			bRes = m_Log.Register(szLogPath);
			if (!bRes) break;
		}

		bRes = _OpenSCM(SC_MANAGER_CREATE_SERVICE);
		if (!bRes) break;

		m_hSvc = CreateService(
					m_hSvcMgr,
					m_szDefName,
					m_szDefDispName,
					GENERIC_WRITE,
					SERVICE_WIN32_OWN_PROCESS,
					SERVICE_AUTO_START,
					SERVICE_ERROR_NORMAL,
					m_szFileName,
					NULL,
					NULL,
					NULL,
					NULL,	// use LocalSystem account
					NULL
					);
		if (!m_hSvc) {
			m_dwError = GetLastError();
			_HandleError(m_dwError);
			bRes = false;
			break;
		}

		SERVICE_DESCRIPTION	sd;

		sd.lpDescription = const_cast<LPTSTR>(m_szDefDesc);
		fRes = ChangeServiceConfig2(m_hSvc, SERVICE_CONFIG_DESCRIPTION, (LPVOID) &sd);
		if (!fRes) {
			m_dwError = GetLastError();
			_HandleError(m_dwError);
			break;
		}

		bRes = true;

	} while (FALSE);

	if (!bRes) {
		m_Log.Unregister();
	}

	_CloseSCM();
	_Close();

	return bRes;
}

bool NSvcControl::Uninstall()
{
	bool	bRes = false;
	BOOL	fRes = FALSE;
	SERVICE_STATUS	SvcStat = {0};

	do {
		bRes = _OpenSCM(SC_MANAGER_ALL_ACCESS);
		if (!bRes) break;

		bRes = _Open(m_szDefName, SERVICE_ALL_ACCESS);
		if (!bRes) break;

		bRes = _QueryStatus(&SvcStat);
		if (!bRes) break;

		m_dwError = 0;
		switch (SvcStat.dwCurrentState)
		{
		case SERVICE_CONTINUE_PENDING:
			DBG_PRINTF(_T("%s: In continue pending state!\r\n"), FUNCT_NAME_STR);
			break;

		case SERVICE_PAUSE_PENDING:
			DBG_PRINTF(_T("%s: In pause pending state!\r\n"), FUNCT_NAME_STR);
			break;

		case SERVICE_PAUSED:
			bRes = _Control(SERVICE_CONTROL_CONTINUE);
			if (bRes) {
				bRes = _Control(SERVICE_CONTROL_STOP);
			}
			break;

		case SERVICE_RUNNING:
			bRes = _Control(SERVICE_CONTROL_STOP);
			break;

		case SERVICE_START_PENDING:
			DBG_PRINTF(_T("%s: In start pending state!\r\n"), FUNCT_NAME_STR);
			break;

		case SERVICE_STOP_PENDING:
			DBG_PRINTF(_T("%s: In stop pending state!\r\n"), FUNCT_NAME_STR);
			break;

		case SERVICE_STOPPED:
			bRes = true;
			break;

		default:
			DBG_PRINTF(_T("%s: Unknown service state! (%d)\r\n"), FUNCT_NAME_STR, SvcStat.dwCurrentState);
			break;
		}

		if (!bRes) {
			if (m_dwError == ERROR_SUCCESS) {
				m_dwError = ERROR_INVALID_STATE;
				_HandleError(m_dwError);
			}
			break;
		}

		fRes = DeleteService(m_hSvc);
		if (!fRes) {
			m_dwError = GetLastError();
			bRes = false;
			_HandleError(m_dwError);
			break;
		}

		m_Log.Unregister();

		bRes = true;
	} while (FALSE);

	_Close();
	_CloseSCM();

	return bRes;
}

bool NSvcControl::Start()
{
	bool	bRes = false;
	BOOL	fRes = FALSE;

	do {
		bRes = _OpenSCM(SC_MANAGER_ALL_ACCESS);
		if (!bRes) break;

		bRes = _Open(m_szDefName, SERVICE_START);
		if (!bRes) break;

		fRes = StartService(m_hSvc, 0, NULL);
		if (!fRes) {
			m_dwError = GetLastError();
			_HandleError(m_dwError);
			break;
		}

		bRes = true;
	} while (FALSE);

	_CloseSCM();
	_Close();

	return bRes;
}

bool NSvcControl::Stop()
{
	bool			bRes = false;
	BOOL			fRes = FALSE;
	SERVICE_STATUS	SvcStat = {0};

	do {
		bRes = _OpenSCM(SC_MANAGER_ALL_ACCESS);
		if (!bRes) break;

		bRes = _Open(m_szDefName, SERVICE_STOP);
		if (!bRes) break;

		fRes = ControlService(m_hSvc, SERVICE_CONTROL_STOP, &SvcStat);
		if (!fRes) {
			m_dwError = GetLastError();
			_HandleError(m_dwError);
			break;
		}

		bRes = true;
	} while (FALSE);

	_CloseSCM();
	_Close();

	return bRes;
}

bool NSvcControl::Pause()
{
	bool			bRes = false;
	BOOL			fRes = FALSE;
	SERVICE_STATUS	SvcStat = {0};

	do {
		bRes = _OpenSCM(SC_MANAGER_ALL_ACCESS);
		if (!bRes) break;

		bRes = _Open(m_szDefName, SERVICE_PAUSE_CONTINUE);
		if (!bRes) break;

		fRes = ControlService(m_hSvc, SERVICE_CONTROL_PAUSE, &SvcStat);
		if (!fRes) {
			m_dwError = GetLastError();
			_HandleError(m_dwError);
			break;
		}

		bRes = true;
	} while (FALSE);

	_CloseSCM();
	_Close();

	return bRes;
}

bool NSvcControl::Resume()
{
	bool			bRes = false;
	BOOL			fRes = FALSE;
	SERVICE_STATUS	SvcStat = {0};

	do {
		bRes = _OpenSCM(SC_MANAGER_ALL_ACCESS);
		if (!bRes) break;

		bRes = _Open(m_szDefName, SERVICE_PAUSE_CONTINUE);
		if (!bRes) break;

		fRes = ControlService(m_hSvc, SERVICE_CONTROL_CONTINUE, &SvcStat);
		if (!fRes) {
			m_dwError = GetLastError();
			_HandleError(m_dwError);
			break;
		}

		bRes = true;
	} while (FALSE);

	_CloseSCM();
	_Close();

	return bRes;
}

void NSvcControl::_HandleError(DWORD dwError)
{
	LPTSTR		pErrStr = NULL;
	DWORD		dwRes = FALSE;
	size_t		stLen = 0;

	if (dwError == 0) {
		return;
	}

	RtlZeroMemory(m_szErrBuf, sizeof(m_szErrBuf));
	dwRes = FormatMessage(
						FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_FROM_SYSTEM |
						FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						dwError,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						reinterpret_cast<LPTSTR>(&pErrStr),
						0,
						NULL
						);
	if (dwRes > 0) {
		stLen = lstrlen(pErrStr);
		pErrStr[stLen - 1] = _T('\0');
		pErrStr[stLen - 2] = _T('\0');
		StringCbPrintf(
					m_szErrBuf, 
					sizeof(m_szErrBuf), 
					_T("%s"), 
					reinterpret_cast<LPTSTR>(pErrStr)
					);
		LocalFree(reinterpret_cast<HLOCAL>(pErrStr));
	} else {
		StringCbPrintf(m_szErrBuf, sizeof(m_szErrBuf), _T("Code %d"), dwError);
	}

	OnError(&m_szErrBuf[0]);

	DBG_PRINTF(_T("%s\r\n"), m_szErrBuf);
}

bool NSvcControl::IsInstalled(bool & bResult)
{
	bool	bRes = false;

	do {
		bRes = _OpenSCM(SC_MANAGER_ENUMERATE_SERVICE);
		if (!bRes) break;

		bRes = _Open(m_szDefName, SERVICE_QUERY_STATUS);
		if (!bRes) {
			if (m_dwError == ERROR_SERVICE_DOES_NOT_EXIST) {
				bRes = true;
				bResult = false;
			}
		} else {
			bResult = true;
		}
	} while (FALSE);

	_Close();
	_CloseSCM();

	return bRes;
}

bool NSvcControl::GetStatus(NSvcControl::SvcStatus &Status)
{
	Status = NSvcControl::Unknown;

	bool			bRes = false;
	SERVICE_STATUS	SvcStat = {0};

	do {
		bRes = _OpenSCM(SC_MANAGER_ENUMERATE_SERVICE);
		if (!bRes) break;

		bRes = _Open(m_szDefName, SERVICE_QUERY_STATUS);
		if (!bRes) break;

		bRes = _QueryStatus(&SvcStat);
		if (!bRes) break;

		Status = static_cast<NSvcControl::SvcStatus>(SvcStat.dwCurrentState);
		bRes = true;
	} while (FALSE);

	_Close();
	_CloseSCM();

	return bRes;
}