/*
 * File    : Protocol.cpp
 * Remark  : Protocol implementation.
 *
 */

#include "Protocol.h"
#include "..\NTerminal\Debug.h"

/*
 * BEGIN NSecurityDescriptor
 */
NSecurityDescriptor::NSecurityDescriptor()
{
	m_pSecDesc		= NULL;
	m_szDefDesc		=
		_T("D:")					// Discretionary ACL
		_T("(D;OICI;GA;;;BG)")		// Deny access to built-in guess
		_T("(D;OICI;GA;;;AN)")		// Deny access to anonymous logon
		//_T("(A;OICI;GRGWGX;;;AU)")	// Allow READ/WRITE/EXECUTE access to authenticated user
		_T("(A;OICI;GA;;;AU)")		// Allow full control to authenticated user - dangerous!!!
		_T("(A;OICI;GA;;;BA)");		// Allow full control to administrator
}

NSecurityDescriptor::~NSecurityDescriptor()
{
	if (m_pSecDesc) {
		LocalFree(reinterpret_cast<HLOCAL>(m_pSecDesc));
	}
}

bool NSecurityDescriptor::Build(LPCTSTR szDesc)
{
	TCHAR * szDescVar = NULL;

	if (m_pSecDesc) return true;

	if (szDesc == NULL) {
		szDescVar = const_cast<TCHAR *>(m_szDefDesc);
	} else {
		szDescVar = const_cast<TCHAR *>(szDesc);
	}
	
	BOOL fRes =
		ConvertStringSecurityDescriptorToSecurityDescriptor(
										reinterpret_cast<LPCTSTR>(szDescVar),
										SDDL_REVISION_1,
										&m_pSecDesc,
										NULL
										);
	DBGIF_PRINTF((fRes == FALSE),
		_T("%s: Can't create security descriptor! (%d)\r\n"), FUNCT_NAME_STR, GetLastError());
	
	return ((fRes) ? true : false);
}

PSECURITY_DESCRIPTOR NSecurityDescriptor::GetPtr()
{
	return m_pSecDesc;
}

void NSecurityDescriptor::Free()
{
	if (m_pSecDesc) {
		LocalFree(reinterpret_cast<HLOCAL>(m_pSecDesc));
		m_pSecDesc = NULL;
	}
}
/*
 * END of NSecurityDescriptor
 */

/*
 * BEGIN NSynch implementation
 */
NSynch::NSynch():
	m_szMutexNamePrefix(_T("Global\\NPortSvc_Mutex_")),
	m_szEventNamePrefix(_T("Global\\NPortSvc_Event_")),
	m_szSemNamePrefix(_T("Global\\NPortSvc_Semap_")),
	m_dwError(0)
{
	RtlZeroMemory(&m_hMutexes, (sizeof(HANDLE) * NPROTO_MTX_ID__MAX));
	RtlZeroMemory(&m_hEvents, (sizeof(HANDLE) * NPROTO_EVT_ID__MAX));
	RtlZeroMemory(&m_hSemaps, (sizeof(HANDLE) * NPROTO_SEMAP_ID__MAX));
}

NSynch::~NSynch()
{
}

void NSynch::_Destroy()
{
	for (int i = 0; i < NPROTO_MTX_ID__MAX; i++) {
		if (m_hMutexes[i]) {
			CloseHandle(m_hMutexes[i]);
			m_hMutexes[i] = NULL;
		}
	}
	for (int i = 0; i < NPROTO_EVT_ID__MAX; i++) {
		if (m_hEvents[i]) {
			CloseHandle(m_hEvents[i]);
			m_hEvents[i] = NULL;
		}
	}
	for (int i = 0; i < NPROTO_SEMAP_ID__MAX; i++) {
		if (m_hSemaps[i]) {
			CloseHandle(m_hSemaps[i]);
			m_hSemaps[i] = NULL;
		}
	}
}

DWORD NSynch::GetError()
{
	return m_dwError;
}

bool NSynch::AcquireDevEventLock(DWORD dwMSecTimeOut, LPDWORD pdwResult)
{
	DWORD dwRes = WAIT_FAILED;
	dwRes = WaitForSingleObject(m_hMutexes[NPROTO_MTX_ID__DEV_EVT], dwMSecTimeOut);
	if (dwRes == WAIT_FAILED) {
		m_dwError = GetLastError();
		return false;
	}
	if (pdwResult) *pdwResult = dwRes;
	return true;
}

bool NSynch::ReleaseDevEventLock()
{
	BOOL fRes = ReleaseMutex(m_hMutexes[NPROTO_MTX_ID__DEV_EVT]);
	if (!fRes) m_dwError = GetLastError();
	return ((fRes) ? true : false);
}

bool NSynch::AcquireReadLock(DWORD dwMSecTimeOut, LPDWORD pdwResult)
{
	DWORD dwRes = WaitForSingleObject(m_hMutexes[NPROTO_MTX_ID__READ], dwMSecTimeOut);
	if (dwRes == WAIT_FAILED) {
		m_dwError = GetLastError();
		return false;
	}
	if (pdwResult) *pdwResult = dwRes;
	return true;
}

bool NSynch::ReleaseReadLock()
{
	BOOL fRes = ReleaseMutex(m_hMutexes[NPROTO_MTX_ID__READ]);
	if (!fRes) m_dwError = GetLastError();
	return ((fRes) ? true : false);
}

bool NSynch::AcquireWriteLock(DWORD dwMSecTimeOut, LPDWORD pdwResult)
{
	DWORD dwRes = WaitForSingleObject(m_hMutexes[NPROTO_MTX_ID__WRITE], dwMSecTimeOut);
	if (dwRes == WAIT_FAILED) {
		m_dwError = GetLastError();
		return false;
	}
	if (pdwResult) *pdwResult = dwRes;
	return true;
}

bool NSynch::ReleaseWriteLock()
{
	BOOL fRes = ReleaseMutex(m_hMutexes[NPROTO_MTX_ID__WRITE]);
	if (!fRes) {
		m_dwError = GetLastError();
		return false;
	}
	return true;
}

bool NSynch::AcquireReadWriteLock(DWORD dwMSecTimeOut, LPDWORD pdwReadResult, LPDWORD pdwWriteResult)
{
	bool	bRes = AcquireReadLock(dwMSecTimeOut, pdwReadResult);
	if (bRes) {
		bRes = AcquireWriteLock(dwMSecTimeOut, pdwWriteResult);
		if (!bRes) {
			ReleaseReadLock();
		}
	}

	return bRes;
}

bool NSynch::ReleaseReadWriteLock()
{
	bool	bResR = ReleaseReadLock();
	bool	bResW = ReleaseWriteLock();

	return (bResR & bResW);
}

bool NSynch::SetReadRequest()
{
	BOOL fRes = SetEvent(m_hEvents[NPROTO_EVT_ID__READ_REQ]);
	if (!fRes) {
		m_dwError = GetLastError();
		return false;
	}
	return true;
}

bool NSynch::ResetReadRequest()
{
	BOOL fRes = ResetEvent(m_hEvents[NPROTO_EVT_ID__READ_REQ]);
	if (!fRes) {
		m_dwError = GetLastError();
		return false;
	}
	return true;
}

bool NSynch::SetReadComplete()
{
	BOOL fRes = SetEvent(m_hEvents[NPROTO_EVT_ID__READ_CMPL]);
	if (!fRes) {
		m_dwError = GetLastError();
		return false;
	}
	return true;
}

bool NSynch::ResetReadComplete()
{
	BOOL fRes = ResetEvent(m_hEvents[NPROTO_EVT_ID__READ_CMPL]);
	if (!fRes) {
		m_dwError = GetLastError();
		return false;
	}
	return true;
}

bool NSynch::SetWriteRequest()
{
	BOOL fRes = SetEvent(m_hEvents[NPROTO_EVT_ID__WRITE_REQ]);
	if (!fRes) {
		m_dwError = GetLastError();
		return false;
	}
	return true;
}

bool NSynch::ResetWriteRequest()
{
	BOOL fRes = ResetEvent(m_hEvents[NPROTO_EVT_ID__WRITE_REQ]);
	if (!fRes) {
		m_dwError = GetLastError();
		return false;
	}
	return true;
}

bool NSynch::SetWriteCompelete()
{
	BOOL fRes = SetEvent(m_hEvents[NPROTO_EVT_ID__WRITE_CMPL]);
	if (!fRes) {
		m_dwError = GetLastError();
		return false;
	}
	return true;
}

bool NSynch::ResetWriteComplete()
{
	BOOL fRes = ResetEvent(m_hEvents[NPROTO_EVT_ID__WRITE_CMPL]);
	if (!fRes) {
		m_dwError = GetLastError();
		return false;
	}
	return true;
}
/*
 * END of NSynch implementation
 */


/*
 * BEGIN ClientSynch implementation
 */
ClientSynch::ClientSynch()
{
}

ClientSynch::~ClientSynch()
{
	_Destroy();
}

bool ClientSynch::_OpenMutex(DWORD dwMutexId)
{
	TCHAR		szName[256] = {0};
	HRESULT		hr = S_OK;

	hr = StringCbPrintf(szName, sizeof(szName), _T("%s%d"), m_szMutexNamePrefix, dwMutexId);
	DBGIF_PRINTF((hr == S_OK), _T("%s: Generated name: %s\r\n"), FUNCT_NAME_STR, szName);
	if (hr != S_OK) {
		m_dwError = HRESULT_CODE(hr);
		return false;
	}

	m_hMutexes[dwMutexId] = OpenMutex(MUTEX_ALL_ACCESS, FALSE, szName);
	if (!m_hMutexes[dwMutexId]) {
		m_dwError = GetLastError();
		DBG_PRINTF(_T("%s: OpenMutex() failed for id %d! (%d)\r\n"), FUNCT_NAME_STR, dwMutexId, m_dwError);
		return false;
	}

	return true;
}

bool ClientSynch::_OpenEvent(DWORD dwEventId)
{
	TCHAR		szName[256] = {0};
	HRESULT		hr = S_OK;
	DWORD		dwAcc = (SYNCHRONIZE | EVENT_MODIFY_STATE);

	hr = StringCbPrintf(szName, sizeof(szName), _T("%s%d"), m_szEventNamePrefix, dwEventId);
	DBGIF_PRINTF((hr == S_OK), _T("%s: Generated name: %s\r\n"), FUNCT_NAME_STR, szName);
	if (hr != S_OK) {
		m_dwError = HRESULT_CODE(hr);
		return false;
	}

	m_hEvents[dwEventId] = OpenEvent(dwAcc, FALSE, szName);
	if (!m_hEvents[dwEventId]) {
		m_dwError = GetLastError();
		DBG_PRINTF(_T("%s: OpenEvent() failed for id %d! (%d)\r\n"), FUNCT_NAME_STR, dwEventId, m_dwError);
		return false;
	}

	return true;
}

bool ClientSynch::_OpenSemap(DWORD dwSemapId)
{
	TCHAR		szName[256] = {0};
	HRESULT		hr = S_OK;
	DWORD		dwAcc = (SYNCHRONIZE | SEMAPHORE_MODIFY_STATE);

	hr = StringCbPrintf(szName, sizeof(szName), _T("%s%d"), m_szSemNamePrefix, dwSemapId);
	DBGIF_PRINTF((hr == S_OK), _T("%s: Generated name: %s\r\n"), FUNCT_NAME_STR, szName);
	if (hr != S_OK) {
		m_dwError = HRESULT_CODE(hr);
		return false;
	}

	m_hSemaps[dwSemapId] = OpenSemaphore(dwAcc, FALSE, szName);
	if (!m_hSemaps[dwSemapId]) {
		m_dwError = GetLastError();
		DBG_PRINTF(_T("%s: OpenSemaphore() failed for id %d! (%d)\r\n"), FUNCT_NAME_STR, dwSemapId, m_dwError);
		return false;
	}

	return true;
}

bool ClientSynch::Open()
{
	if (m_hMutexes[0]) return true;

	BOOL	fRes = FALSE;

	bool bRes = false;
	do {
		for (DWORD d = 0; d < NPROTO_MTX_ID__MAX; d++)
		{
			bRes = _OpenMutex(d);
			if (!bRes) break;
		}
		if (!bRes) break;

		for (DWORD d = 0; d < NPROTO_EVT_ID__MAX; d++)
		{
			bRes = _OpenEvent(d);
			if (!bRes) break;
		}
		if (!bRes) break;

		for (DWORD d = 0; d < NPROTO_SEMAP_ID__MAX; d++)
		{
			bRes = _OpenSemap(d);
			if (!bRes) break;
		}
		if (!fRes) break;

	} while (FALSE);

	if (!bRes)
	{
		_Destroy();
	}

	return bRes;
}

void ClientSynch::Close()
{
	_Destroy();
}

bool ClientSynch::AcquireDevEventLock()
{
	return NSynch::AcquireDevEventLock(INFINITE, NULL);
}

bool ClientSynch::ReleaseDevEvent()
{
	BOOL	fRes = FALSE;

	fRes = ReleaseSemaphore(m_hSemaps[NPROTO_SEMAP_ID__DEV_EVT], 1, NULL);
	if (!fRes) {
		m_dwError = GetLastError();
	}

	return ((fRes) ? true : false);
}

DWORD ClientSynch::WaitForDevEvent(DWORD dwMSecTimeOut)
{
	DWORD dwRes = WaitForSingleObject(m_hSemaps[NPROTO_SEMAP_ID__DEV_EVT], dwMSecTimeOut);
	if (dwRes == WAIT_FAILED) m_dwError = GetLastError();
	return dwRes;
}

bool ClientSynch::RequestReadAndWait(DWORD dwMSecTimeOut, LPDWORD pdwWaitStat)
{
	DWORD dwRes = WAIT_FAILED;

	dwRes = SignalObjectAndWait(
					m_hEvents[NPROTO_EVT_ID__READ_REQ], 
					m_hEvents[NPROTO_EVT_ID__READ_CMPL], 
					dwMSecTimeOut, 
					FALSE
					);
	if (dwRes == WAIT_FAILED) {
		m_dwError = GetLastError();
		return false;
	}
	if (pdwWaitStat) *pdwWaitStat = dwRes;
	return true;
}

bool ClientSynch::RequestWriteAndWait(DWORD dwMSecTimeOut, LPDWORD pdwWaitStat)
{
	DWORD dwRes = WAIT_FAILED;

	dwRes = SignalObjectAndWait(
					m_hEvents[NPROTO_EVT_ID__WRITE_REQ],
					m_hEvents[NPROTO_EVT_ID__WRITE_CMPL],
					dwMSecTimeOut,
					FALSE
					);
	if (dwRes == WAIT_FAILED) {
		m_dwError = GetLastError();
		return false;
	}
	if (pdwWaitStat) *pdwWaitStat = dwRes;
	return true;
}
/*
 * END of ClientSynch implementation
 */

/*
 * BEGIN ServerSynch implementation
 */
ServerSynch::ServerSynch()
{
	m_MtxSecDesc.Build(NULL);
	m_EvtSecDesc.Build(NULL);
	m_SemSecDesc.Build(NULL);
}

ServerSynch::~ServerSynch()
{
	_Destroy();
}

bool ServerSynch::_CreateMutex(DWORD dwMutexId)
{
	TCHAR	szName[256] = {0};
	HRESULT	hr = S_OK;
	BOOL	fInitOwner = FALSE;

	StringCbPrintf(szName, sizeof(szName), _T("%s%d"), m_szMutexNamePrefix, dwMutexId);
	DBGIF_PRINTF((hr == S_OK), _T("%s: Generated name: %s\r\n"), FUNCT_NAME_STR, szName);
	if (hr != S_OK) {
		m_dwError = HRESULT_CODE(hr);
		return false;
	}

	SECURITY_ATTRIBUTES	sa;
	sa.bInheritHandle		= FALSE;
	sa.lpSecurityDescriptor	= m_MtxSecDesc.GetPtr();
	sa.nLength				= sizeof(SECURITY_ATTRIBUTES);
	m_hMutexes[dwMutexId] = CreateMutex(&sa, fInitOwner, szName);
	if (!m_hMutexes[dwMutexId]) {
		m_dwError = GetLastError();
		if (m_dwError == ERROR_ALREADY_EXISTS) {
			m_hMutexes[dwMutexId] = OpenMutex(MUTEX_ALL_ACCESS, FALSE, szName);
			if (!m_hMutexes[dwMutexId]) {
				m_dwError = GetLastError();
			}
		}
	}
	DBGIF_PRINTF((!m_hMutexes[dwMutexId]), _T("%s: Create/Open mutex failed! (%d)\r\n"), FUNCT_NAME_STR, m_dwError);

	return ((m_hMutexes[dwMutexId]) ? true : false);
}

bool ServerSynch::_CreateEvent(DWORD dwEventId)
{
	TCHAR	szName[256] = {0};
	HRESULT	hr = S_OK;
	BOOL	fManReset = FALSE;
	BOOL	fInitState = FALSE;

	StringCbPrintf(szName, sizeof(szName), _T("%s%d"), m_szEventNamePrefix, dwEventId);
	DBGIF_PRINTF((hr == S_OK), _T("%s: Generated name: %s\r\n"), FUNCT_NAME_STR, szName);
	if (hr != S_OK) {
		m_dwError = HRESULT_CODE(hr);
		return false;
	}

	SECURITY_ATTRIBUTES	sa;
	sa.bInheritHandle		= FALSE;
	sa.lpSecurityDescriptor	= m_EvtSecDesc.GetPtr();
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	m_hEvents[dwEventId]	= CreateEvent(&sa, fManReset, fInitState, szName);
	if (!m_hEvents[dwEventId]) {
		m_dwError = GetLastError();
		if (m_dwError == ERROR_ALREADY_EXISTS) {
			m_hEvents[dwEventId] = OpenEvent(EVENT_ALL_ACCESS, FALSE, szName);
			if (!m_hEvents[dwEventId]) {
				m_dwError = GetLastError();
			}
		}
	}

	DBGIF_PRINTF((m_hEvents[dwEventId] == NULL), 
		_T("%s: CreateEvent() failed for id: %d! (%d)\r\n"), FUNCT_NAME_STR, dwEventId, m_dwError);

	return ((m_hEvents[dwEventId] == NULL) ? false : true);
}

bool ServerSynch::_CreateSemaphore(DWORD dwSemapId)
{
	TCHAR				szName[256] = {0};
	HRESULT				hr = S_OK;
	SECURITY_ATTRIBUTES	sa = {0};
	DWORD				dwMaxSem = 1;

	StringCbPrintf(szName, sizeof(szName), _T("%s%d"), m_szSemNamePrefix, dwSemapId);
	DBGIF_PRINTF((hr == S_OK), _T("%s: Generated name: %s\r\n"), FUNCT_NAME_STR, szName);
	if (hr != S_OK) {
		m_dwError = HRESULT_CODE(hr);
		return false;
	}

	sa.bInheritHandle		= FALSE;
	sa.lpSecurityDescriptor	= m_SemSecDesc.GetPtr();
	sa.nLength				= sizeof(SECURITY_DESCRIPTOR);

	switch (dwSemapId)
	{
	case NPROTO_SEMAP_ID__DEV_EVT:
		dwMaxSem = NPROTO__MAX_DEV_EVENTS;
		break;

	default:
		break;
	}

	m_hSemaps[dwSemapId] = CreateSemaphore(&sa, 0, dwMaxSem, szName);
	if (!m_hSemaps[dwSemapId]) {
		m_dwError = GetLastError();
		if (m_dwError == ERROR_ALREADY_EXISTS) {
			m_hSemaps[dwSemapId] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, szName);
			if (!m_hSemaps[dwSemapId]) {
				m_dwError = GetLastError();
			}
		}
	}
	DBGIF_PRINTF((m_hSemaps[dwSemapId] == NULL), _T("%s: Create/Open semaphore failed! (%d)\r\n"), FUNCT_NAME_STR, m_dwError);

	return ((m_hSemaps[dwSemapId]) ? true : false);
}

bool ServerSynch::Create()
{
	if (m_hEvents[0]) return true;

	bool	bRes = false;
	do {
		for (DWORD d = 0; d < NPROTO_MTX_ID__MAX; d++)
		{
			bRes = _CreateMutex(d);
			if (!bRes) break;
		}
		if (!bRes) break;

		for (DWORD d = 0; d < NPROTO_EVT_ID__MAX; d++)
		{
			bRes = _CreateEvent(d);
			if (!bRes) break;
		}
		if (!bRes) break;

		for (DWORD d = 0; d < NPROTO_SEMAP_ID__MAX; d++)
		{
			bRes = _CreateSemaphore(d);
			if (!bRes) break;
		}
		if (!bRes) break;

	} while (FALSE);

	if (!bRes)
	{
		_Destroy();
	}

	return bRes;
}

void ServerSynch::Destroy()
{
	_Destroy();
}

bool ServerSynch::AcquireLock()
{
	bool	bRes = false;

	bRes = AcquireDevEventLock(INFINITE, NULL);
	if (bRes) {
		bRes = AcquireReadWriteLock(INFINITE, NULL, NULL);
		if (!bRes) {
			ReleaseDevEventLock();
		}
	}

	return bRes;
}

bool ServerSynch::ReleaseLock()
{
	bool	bRes1 = ReleaseDevEventLock();
	bool	bRes2 = ReleaseReadWriteLock();

	return ((bRes1 & bRes2));
}

bool ServerSynch::ReleaseDevEvent(LONG lNumberOfEvents, PLONG pPrevNumberOfEvents)
{
	BOOL fRes = FALSE;

	fRes = ReleaseSemaphore(m_hSemaps[NPROTO_SEMAP_ID__DEV_EVT], lNumberOfEvents, pPrevNumberOfEvents);
	if (!fRes) {
		m_dwError = GetLastError();
	}

	return ((fRes) ? true : false);
}

bool ServerSynch::SetReadCompletion()
{
	BOOL fRes = FALSE;

	fRes = SetEvent(m_hEvents[NPROTO_EVT_ID__READ_CMPL]);
	if (!fRes) {
		m_dwError = GetLastError();
		return false;
	}
	return true;
}

bool ServerSynch::SetWriteCompletion()
{
	BOOL fRes = FALSE;
	
	fRes = SetEvent(m_hEvents[NPROTO_EVT_ID__WRITE_CMPL]);
	if (!fRes) {
		m_dwError = GetLastError();
		return false;
	}
	return true;
}

bool ServerSynch::WaitForReadWriteRequest(DWORD dwMSecTimeOut, ServerSynch::Request & RequestResult)
{
	DWORD	dwRes = 0;
	bool	bRes = false;

	dwRes = WaitForMultipleObjects(2, &(m_hEvents[NPROTO_EVT_ID__READ_REQ]), FALSE, dwMSecTimeOut);
	switch (dwRes)
	{
	case (WAIT_OBJECT_0 + 0):
		RequestResult = ServerSynch::Req_Read;
		bRes = true;
		break;

	case (WAIT_OBJECT_0 + 1):
		RequestResult = ServerSynch::Req_Write;
		bRes = true;
		break;

	case WAIT_TIMEOUT:
		RequestResult = ServerSynch::Req_None;
		m_dwError = ERROR_TIMEOUT;
		break;

	default:
		RequestResult = ServerSynch::Req_None;
		m_dwError = GetLastError();
		break;
	}

	return bRes;
}
/*
 * END of ServerSynch implementation
 */


/*
 * BEGIN NFileMapping implementation
 */
NFileMapping::NFileMapping():
	m_dwSize(NPROTO_MEM_SIZE),
	m_szMapName(_T("Global\\NPortSvc_MapFile")),
	m_hMapFile(NULL),
	m_pMem(NULL),
	m_dwEventErr(0),
	m_dwReadErr(0),
	m_dwWriteErr(0),
	m_dwErr(0)
{
}

NFileMapping::~NFileMapping()
{
}

DWORD NFileMapping::GetSize()
{
	return m_dwSize;
}

DWORD NFileMapping::GetEventError()
{
	return m_dwEventErr;
}

DWORD NFileMapping::GetError()
{
	return m_dwErr;
}

DWORD NFileMapping::GetReadError()
{
	return m_dwReadErr;
}

DWORD NFileMapping::GetWriteError()
{
	return m_dwWriteErr;
}

void NFileMapping::_Destroy()
{
	BOOL fRes = TRUE;

	if (m_pMem) {
		fRes = UnmapViewOfFile(m_pMem);
		DBGIF_PRINTF((fRes == FALSE), 
			_T("%s: Warning: UnmapViewOfFile() failed! (%d)\r\n"), FUNCT_NAME_STR, GetLastError());
		m_pMem = NULL;
	}

	if (m_hMapFile) {
		fRes = CloseHandle(m_hMapFile);
		m_hMapFile = NULL;
	}
}

PNPROTO_DATA NFileMapping::_GetProtoDataPtr()
{
	return (reinterpret_cast<PNPROTO_DATA>(m_pMem));
}
/*
 * END of NFileMapping implementation
 */


/*
 * BEGIN ClientFileMapping implementation
 */
ClientFileMapping::ClientFileMapping()
{
}

ClientFileMapping::~ClientFileMapping()
{
	_Destroy();
}

bool ClientFileMapping::_Open()
{
	m_hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, m_szMapName);
	if (!m_hMapFile) {
		m_dwErr = GetLastError();
		DBG_PRINTF(_T("%s: OpenFileMapping() failed! (%d)\r\n"), FUNCT_NAME_STR, m_dwErr);
		return false;
	}

	m_pMem = MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, m_dwSize);
	if (!m_pMem) {
		m_dwErr = GetLastError();
		DBG_PRINTF(_T("%s: MapViewOfFile() failed! (%d)\r\n"), FUNCT_NAME_STR, m_dwErr);
		return false;
	}

	return true;
}

bool ClientFileMapping::Open()
{
	if (m_hMapFile) return true;

	bool bRes = _Open();

	if (!bRes)
	{
		_Destroy();
	}

	return bRes;
}

void ClientFileMapping::Close()
{
	_Destroy();
}

bool ClientFileMapping::RequestDevEvent(ULONG &uIndex, ULONG &ulInitialValue)
{
	PNPROTO_DATA		pProt = _GetProtoDataPtr();
	PNPROTO_EVENT_SLOT	pSlot = NULL;

	if (!pProt) {
		m_dwEventErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	uIndex = NPROTO__MAX_DEV_EVENTS;
	m_dwEventErr = 0;
	__try {
		if (pProt->SvcStatus == NPROTO_SVC_STATUS__DISABLED) {
			m_dwEventErr = ERROR_SERVICE_NOT_ACTIVE;
		} else {
			ulInitialValue = pProt->Event.InitialStatus;
			pSlot = &(pProt->Event.Slots[0]);
			for (ULONG u = 0; u < NPROTO__MAX_DEV_EVENTS; u++)
			{
				if (pSlot->Status == NPROTO_EVENT__DISABLED)
				{
					pSlot->Pid		= GetCurrentProcessId();
					pSlot->Status	= NPROTO_EVENT__ENABLED;
					uIndex			= u;
					pProt->Event.Counter++;
					break;
				} else {
					if (pSlot->Pid == GetCurrentProcessId()) {
						m_dwEventErr = ERROR_ALREADY_REGISTERED;
						break;
					}
				}
				pSlot++;
			}

			if (uIndex < NPROTO__MAX_DEV_EVENTS) {
				m_dwEventErr = ERROR_SUCCESS;
			} else {
				if (m_dwEventErr == 0) 
					m_dwEventErr = ERROR_OUTOFMEMORY;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwEventErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred!!! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwEventErr);
	}

	return ((m_dwEventErr == ERROR_SUCCESS) ? true : false);
}

bool ClientFileMapping::GetDevEventStatus(ULONG uIndex, ULONG &ulEventStatus, BOOL fEnableStatus)
{
	PNPROTO_DATA		pProt = _GetProtoDataPtr();
	PNPROTO_EVENT_SLOT	pSlot = NULL;

	if (!pProt) {
		m_dwEventErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if (uIndex >= NPROTO__MAX_DEV_EVENTS) {
		m_dwEventErr = ERROR_INVALID_INDEX;
		return false;
	}

	__try {
		if (pProt->SvcStatus == NPROTO_SVC_STATUS__DISABLED) {
			m_dwEventErr = ERROR_SERVICE_NOT_ACTIVE;
		} else {
			pSlot = &(pProt->Event.Slots[0]);
			pSlot += uIndex;
			ulEventStatus = pSlot->Status;
			if (fEnableStatus) {
				pSlot->Status = NPROTO_EVENT__ENABLED;
			}

			m_dwEventErr = ERROR_SUCCESS;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwEventErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred!!! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwEventErr);
	}

	return ((m_dwEventErr == ERROR_SUCCESS) ? true : false);
}

bool ClientFileMapping::GetDevEventStatus(ULONG uIndex, ULONG &ulEventStatus)
{
	PNPROTO_DATA		pProt = _GetProtoDataPtr();
	PNPROTO_EVENT_SLOT	pSlot = NULL;

	if (!pProt) {
		m_dwEventErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if (uIndex >= NPROTO__MAX_DEV_EVENTS) {
		m_dwEventErr = ERROR_INVALID_INDEX;
		return false;
	}

	__try {
		if (pProt->SvcStatus == NPROTO_SVC_STATUS__DISABLED) {
			m_dwEventErr = ERROR_SERVICE_NOT_ACTIVE;
		} else {
			pSlot			= &(pProt->Event.Slots[0]);
			pSlot			+= uIndex;
			ulEventStatus	= pSlot->Status;
			if ((pSlot->Status == NPROTO_EVENT__DEV_ARRIVAL) ||
				(pSlot->Status == NPROTO_EVENT__DEV_REMOVAL))
			{
				pSlot->Status	= NPROTO_EVENT__ENABLED;
			}

			m_dwEventErr = ERROR_SUCCESS;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwEventErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred!!! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwEventErr);
	}

	return ((m_dwEventErr == ERROR_SUCCESS) ? true : false);
}

bool ClientFileMapping::DisableDevEvent(ULONG ulIndex)
{
	PNPROTO_DATA		pProt = _GetProtoDataPtr();
	PNPROTO_EVENT_SLOT	pSlot = NULL;

	if (!pProt) {
		m_dwEventErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if (ulIndex >= NPROTO__MAX_DEV_EVENTS) {
		m_dwEventErr = ERROR_INVALID_INDEX;
		return false;
	}

	__try {
		if (pProt->SvcStatus == NPROTO_SVC_STATUS__DISABLED) {
			m_dwEventErr = ERROR_SERVICE_NOT_ACTIVE;
		} else {
			pSlot			= &(pProt->Event.Slots[0]);
			pSlot			+= ulIndex;
			pSlot->Status	= NPROTO_EVENT__DISABLED;
			pProt->Event.Counter--;

			m_dwEventErr = ERROR_SUCCESS;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwEventErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred!!! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwEventErr);
	}

	return ((m_dwEventErr == ERROR_SUCCESS) ? true : false);
}

bool ClientFileMapping::RequestRead(ULONG ulRequestedLength)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();

	if (!pProt) {
		m_dwReadErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if (ulRequestedLength == 0) {
		m_dwReadErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	if (ulRequestedLength > (NPROTO_REQUEST_DATA_SIZE - 2))
	{
		m_dwReadErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	PUCHAR	pBuf = NULL;

	__try {
		if (pProt->SvcStatus == NPROTO_SVC_STATUS__DISABLED) {
			m_dwReadErr = ERROR_SERVICE_NOT_ACTIVE;
		} else {
			pProt->Read.Pid		= GetCurrentProcessId();
			pProt->Read.Tid		= GetCurrentThreadId();
			pProt->Read.Length	= ulRequestedLength;
			pProt->Read.Result	= NPROTO_REQUEST_DEF_RESULT;
			pBuf = &(pProt->Read.Data[0]);
			RtlZeroMemory(pBuf, NPROTO_REQUEST_DATA_SIZE);

			m_dwReadErr	= ERROR_SUCCESS;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwReadErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occured!!! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwReadErr);
	}

	return ((m_dwReadErr == ERROR_SUCCESS) ? true : false);
}

bool ClientFileMapping::Read(PUCHAR pBuffer, ULONG uBufferLength, ULONG & uRead)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();
	PUCHAR			pSrc = NULL;

	if (!pProt) {
		m_dwReadErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if ((!pBuffer) || (uBufferLength == 0)) {
		m_dwReadErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	if (uBufferLength > m_dwSize) {
		m_dwReadErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	__try {
		if (pProt->SvcStatus == NPROTO_SVC_STATUS__DISABLED) {
			m_dwReadErr = ERROR_SERVICE_NOT_ACTIVE;
		} else {
			pSrc = &(pProt->Read.Data[0]);
			RtlCopyMemory(pBuffer, pSrc, uBufferLength);
			uRead = pProt->Read.Length;

			m_dwReadErr	= ERROR_SUCCESS;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwReadErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred!!! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwReadErr);
	}

	return ((m_dwReadErr == ERROR_SUCCESS) ? true : false);
}

bool ClientFileMapping::GetReadResult(ULONG & ulResult)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();
	
	if (!pProt) {
		m_dwReadErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	__try {
		if (pProt->SvcStatus == NPROTO_SVC_STATUS__DISABLED) {
			m_dwReadErr = ERROR_SERVICE_NOT_ACTIVE;
		} else {
			ulResult = pProt->Read.Result;
			m_dwReadErr = ERROR_SUCCESS;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwReadErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred! (0x%X)\r\n"), m_dwReadErr);
	}

	return ((m_dwReadErr == ERROR_SUCCESS) ? true : false);
}

bool ClientFileMapping::GetReadResult(PULONG pulResult)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();
	
	if (!pProt) {
		m_dwReadErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if (!pulResult) {
		m_dwReadErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	__try {
		if (pProt->SvcStatus == NPROTO_SVC_STATUS__DISABLED) {
			m_dwReadErr = ERROR_SERVICE_NOT_ACTIVE;
		} else {
			*pulResult = pProt->Read.Result;
			m_dwReadErr = ERROR_SUCCESS;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwReadErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred! (0x%X)\r\n"), m_dwReadErr);
	}

	return ((m_dwReadErr == ERROR_SUCCESS) ? true : false);
}

bool ClientFileMapping::Write(PUCHAR pData, ULONG uDataLength)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();
	PUCHAR			pDest = NULL;
	ULONG			uLength = 0;

	if (!pProt) {
		m_dwWriteErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if ((!pData) || (uDataLength == 0)) {
		m_dwWriteErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	uLength = (uDataLength < NPROTO_REQUEST_DATA_SIZE) ? uDataLength : NPROTO_REQUEST_DATA_SIZE;

	__try {
		if (pProt->SvcStatus == NPROTO_SVC_STATUS__DISABLED) {
			m_dwWriteErr = ERROR_SERVICE_NOT_ACTIVE;
		} else {
			pProt->Write.Pid	= GetCurrentProcessId();
			pProt->Write.Tid	= GetCurrentThreadId();
			pProt->Write.Length	= uLength;
			pProt->Write.Result	= NPROTO_REQUEST_DEF_RESULT;

			pDest = &(pProt->Write.Data[0]);
			RtlCopyMemory(pDest, pData, uLength);

			m_dwWriteErr = ERROR_SUCCESS;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwWriteErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occured!!! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwWriteErr);
	}

	return ((m_dwWriteErr == ERROR_SUCCESS) ? true : false);
}

bool ClientFileMapping::GetWriteResult(ULONG &ulResult)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();

	if (!pProt) {
		m_dwWriteErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	__try {
		if (pProt->SvcStatus == NPROTO_SVC_STATUS__ENABLED) {
			ulResult = pProt->Write.Result;
			m_dwWriteErr = ERROR_SUCCESS;
		} else {
			m_dwWriteErr = ERROR_SERVICE_NOT_ACTIVE;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwWriteErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred! (0x%X)\r\n"), m_dwWriteErr);
	}

	return ((m_dwWriteErr == ERROR_SUCCESS) ? true : false);
}

bool ClientFileMapping::GetWriteResult(PULONG pulResult)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();

	if (!pProt) {
		m_dwWriteErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if (!pulResult) {
		m_dwReadErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	__try {
		if (pProt->SvcStatus == NPROTO_SVC_STATUS__ENABLED) {
			*pulResult = pProt->Write.Result;
			m_dwWriteErr = ERROR_SUCCESS;
		} else {
			m_dwWriteErr = ERROR_SERVICE_NOT_ACTIVE;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwWriteErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred! (0x%X)\r\n"), m_dwWriteErr);
	}

	return ((m_dwWriteErr == ERROR_SUCCESS) ? true : false);
}
/*
 * END of ClientFileMapping implementation
 */


/*
 * BEGIN ServerFileMapping implementation
 */
ServerFileMapping::ServerFileMapping()
{
	m_SecDesc.Build(NULL);
}

ServerFileMapping::~ServerFileMapping()
{
	_Destroy();
}

bool ServerFileMapping::_Create()
{
	BOOL					fRes = FALSE;
	SECURITY_ATTRIBUTES		sa = {0};

	sa.bInheritHandle		= FALSE;
	sa.lpSecurityDescriptor	= m_SecDesc.GetPtr();
	sa.nLength				= sizeof(SECURITY_ATTRIBUTES);

	m_hMapFile = CreateFileMapping(
						INVALID_HANDLE_VALUE,
						&sa,
						(PAGE_READWRITE | SEC_COMMIT),
						0,
						m_dwSize,
						m_szMapName
						);
	if (!m_hMapFile) {
		m_dwErr = GetLastError();
		if (m_dwErr == ERROR_ALREADY_EXISTS) {
			m_hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, m_szMapName);
			if (!m_hMapFile) {
				m_dwErr = GetLastError();
			}
		}
	}
	DBGIF_PRINTF((m_hMapFile == NULL), _T("%s: Create/Open file maping failed! (%d)\r\n"), FUNCT_NAME_STR, m_dwErr);
	if (!m_hMapFile) return false;

	m_pMem = MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, m_dwSize);
	if (!m_pMem) {
		m_dwErr = GetLastError();
		DBG_PRINTF(_T("%s: MapViewOfFile() failed! (%d)\r\n"), FUNCT_NAME_STR, m_dwErr);
	}

	return ((m_dwErr == ERROR_SUCCESS) ? true : false);
}

bool ServerFileMapping::Create()
{
	if (m_hMapFile) return true;

	bool bRes = _Create();

	if (!bRes)
	{
		_Destroy();
	}

	return bRes;
}

void ServerFileMapping::Destroy()
{
	_Destroy();
}

bool ServerFileMapping::Initialize(ULONG ulIntialEvent)
{
	PNPROTO_DATA		pProt = _GetProtoDataPtr();
	PNPROTO_EVENT_SLOT	pSlot = NULL;

	if (!pProt) {
		m_dwErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	__try {
		pProt->SvcStatus			= NPROTO_SVC_STATUS__ENABLED;
		pProt->Event.InitialStatus	= ulIntialEvent;
		pProt->Event.Counter		= 0;
		
		pSlot						= &(pProt->Event.Slots[0]);
		for (ULONG u = 0; u < NPROTO__MAX_DEV_EVENTS; u++)
		{
			pSlot->Pid		= 0;
			pSlot->Status	= NPROTO_EVENT__DISABLED;
			pSlot++;
		}
		
		m_dwErr				= ERROR_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occured!!! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwErr);
	}

	return ((m_dwErr == ERROR_SUCCESS) ? true : false);
}

bool ServerFileMapping::Uninitialize()
{
	PNPROTO_DATA		pProt = _GetProtoDataPtr();

	if (!pProt) {
		m_dwErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	__try {
		pProt->SvcStatus			= NPROTO_SVC_STATUS__DISABLED;
		
		m_dwErr				= ERROR_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occured!!! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwErr);
	}

	return ((m_dwErr == ERROR_SUCCESS) ? true : false);
}

bool ServerFileMapping::SetServiceStatus(ULONG ulSvcStatus)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();

	if (!pProt) {
		m_dwErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if (ulSvcStatus > NPROTO_SVC_STATUS__ENABLED) {
		m_dwErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	__try {
		pProt->SvcStatus	= ulSvcStatus;
		m_dwErr = ERROR_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occured! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwErr);
	}

	return ((m_dwErr == ERROR_SUCCESS) ? true : false);
}

bool ServerFileMapping::SetDevEventsStatus(ULONG ulEventStatus, ULONG & uCount)
{
	PNPROTO_DATA		pProt = _GetProtoDataPtr();
	PNPROTO_EVENT_SLOT	pSlot = NULL;
	HANDLE				hProc = NULL;
	BOOL				fRes = FALSE;

	if (!pProt) {
		m_dwEventErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if ((ulEventStatus > NPROTO_EVENT__DEV_REMOVAL) ||
		(ulEventStatus == NPROTO_EVENT__ENABLED))
	{
		m_dwEventErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	__try {

		pProt->Event.InitialStatus = ulEventStatus;

		if(pProt->Event.Counter > 0) 
		{
			pSlot = &(pProt->Event.Slots[0]);
			for (ULONG u = 0; u < NPROTO__MAX_DEV_EVENTS; u++)
			{
				if (pSlot->Status == NPROTO_EVENT__ENABLED) 
				{
					if (pSlot->Pid != 0) 
					{
						hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pSlot->Pid);
						if (hProc) 
						{
							pSlot->Status = ulEventStatus;
							CloseHandle(hProc);
						} 
						else 
						{
							DBG_PRINTF(_T("%s: PID %d has been closed!\r\n"), FUNCT_NAME_STR, pSlot->Pid);
							pSlot->Pid = 0;
							pSlot->Status = NPROTO_EVENT__DISABLED;
							pProt->Event.Counter--;
						}
					}
				}
				pSlot++;
			}

			uCount = pProt->Event.Counter;
		}

		m_dwEventErr = ERROR_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwEventErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occured!!! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwEventErr);
	}

	return ((m_dwEventErr == ERROR_SUCCESS) ? true : false);
}

bool ServerFileMapping::SetReadResult(DWORD dwErrorCode, ULONG ulLengthResult)
{
	PNPROTO_DATA		pProt = _GetProtoDataPtr();

	if (!pProt) {
		m_dwReadErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	__try {
		pProt->Read.Result	= dwErrorCode;
		pProt->Read.Length	= ulLengthResult;

		m_dwReadErr = ERROR_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwReadErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occured! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwReadErr);
	}

	return ((m_dwReadErr == ERROR_SUCCESS) ? true : false);
}

PUCHAR ServerFileMapping::GetReadBuffer(ULONG &ulRequestedLength)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();
	PUCHAR			pRes = NULL;

	if (!pProt) {
		m_dwReadErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return NULL;
	}

	__try {
		ulRequestedLength	= pProt->Read.Length;
		pRes				= &(pProt->Read.Data[0]);

		m_dwReadErr	= ERROR_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwReadErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occured! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwReadErr);
	}

	return ((m_dwReadErr == ERROR_SUCCESS) ? pRes : NULL);
}

bool ServerFileMapping::GetReadRequestLength(ULONG &ulRequestedLength)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();

	if (!pProt) {
		m_dwReadErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	__try {
		ulRequestedLength	= pProt->Read.Length;

		m_dwReadErr	= ERROR_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwReadErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occured! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwReadErr);
	}

	return ((m_dwReadErr == ERROR_SUCCESS) ? true : false);
}

bool ServerFileMapping::GetReadRequestor(ULONG &ulPid, ULONG &ulTid)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();

	if (!pProt) {
		m_dwReadErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	__try {
		ulPid				= pProt->Read.Pid;
		ulTid				= pProt->Read.Tid;

		m_dwReadErr	= ERROR_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwReadErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occured! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwReadErr);
	}

	return ((m_dwReadErr == ERROR_SUCCESS) ? true : false);
}

bool ServerFileMapping::SetWriteResult(DWORD dwErrorCode, ULONG ulLengthResult)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();

	if (!pProt) {
		m_dwWriteErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	__try {
		pProt->Write.Result	= dwErrorCode;
		pProt->Write.Length	= ulLengthResult;

		m_dwWriteErr = ERROR_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwWriteErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occured! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwWriteErr);
	}

	return ((m_dwWriteErr == ERROR_SUCCESS) ? true : false);
}

PUCHAR ServerFileMapping::GetWriteBuffer(ULONG &ulRequestedLength)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();
	PUCHAR			pRes = NULL;

	if (!pProt) {
		m_dwWriteErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return NULL;
	}

	__try {
		ulRequestedLength	= pProt->Write.Length;
		pRes				= &(pProt->Write.Data[0]);

		m_dwWriteErr = ERROR_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwWriteErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occured! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwWriteErr);
	}

	return ((m_dwWriteErr == ERROR_SUCCESS) ? pRes : NULL);
}

bool ServerFileMapping::GetWriteRequestor(ULONG &ulPid, ULONG &ulTid)
{
	PNPROTO_DATA	pProt = _GetProtoDataPtr();

	if (!pProt) {
		m_dwWriteErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	__try {
		ulPid	= pProt->Write.Pid;
		ulTid	= pProt->Write.Tid;

		m_dwWriteErr = ERROR_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwWriteErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occured! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwWriteErr);
	}

	return ((m_dwWriteErr == ERROR_SUCCESS) ? true : false);
}

bool ServerFileMapping::CopyFromWriteBuffer(PUCHAR pBuffer, ULONG ulBufferLength, ULONG &ulCopiedLength)
{
	PNPROTO_DATA pProt = _GetProtoDataPtr();

	if (!pProt) {
		m_dwWriteErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	ULONG ulLen = (ulBufferLength < NPROTO_REQUEST_DATA_SIZE) ? ulBufferLength : NPROTO_REQUEST_DATA_SIZE;
	PUCHAR pSrc = NULL;

	__try {
		ulLen = (ulLen < pProt->Write.Length) ? ulLen : pProt->Write.Length;
		pSrc = &(pProt->Write.Data[0]);
		RtlCopyMemory(pBuffer, pSrc, ulLen);
		ulCopiedLength = ulLen;

		m_dwWriteErr = ERROR_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwWriteErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred!!! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwWriteErr);
	}

	return ((m_dwWriteErr == ERROR_SUCCESS) ? true : false);
}

bool ServerFileMapping::CopyToReadBuffer(PUCHAR pData, ULONG ulDataLength)
{
	PNPROTO_DATA pProt = _GetProtoDataPtr();

	if (!pProt) {
		m_dwReadErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	ULONG ulLen = (ulDataLength < (NPROTO_REQUEST_DATA_SIZE - 2)) ? ulDataLength : (NPROTO_REQUEST_DATA_SIZE - 2);
	PUCHAR pDst = NULL;

	__try {
		pDst = &(pProt->Read.Data[0]);
		RtlCopyMemory(pDst, pData, ulLen);

		m_dwReadErr = ERROR_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		m_dwReadErr = GetExceptionCode();
		DBG_PRINTF(_T("%s: Exception occurred!!! (0x%X)\r\n"), FUNCT_NAME_STR, m_dwReadErr);
	}

	return ((m_dwReadErr == ERROR_SUCCESS) ? true : false);
}
/*
 * END of ServerFileMapping implementation
 */
