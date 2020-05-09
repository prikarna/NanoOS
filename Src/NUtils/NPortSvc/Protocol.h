/*
 * File    : Protocol.h
 * Remark  : Simple protocol for NanoOS Port service program using file mapping as shared memory 
 *           and event-mutex-semaphore as synchronization tools. Also use the basic concept of 
 *           Client-Server so this protocol can support for multiple access.
 *
 */

#pragma once

#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"
#include "AclApi.h"
#include "Sddl.h"

#ifndef NPORT_SVC_PROTO_DEFINED
#define NPORT_SVC_PROTO_DEFINED

#define NPROTO_MTX_ID__DEV_EVT		0
#define NPROTO_MTX_ID__READ			1
#define NPROTO_MTX_ID__WRITE		2
#define NPROTO_MTX_ID__MAX			3

#define NPROTO_EVT_ID__READ_REQ		0
#define NPROTO_EVT_ID__WRITE_REQ	1
#define NPROTO_EVT_ID__READ_CMPL	2
#define NPROTO_EVT_ID__WRITE_CMPL	3
#define NPROTO_EVT_ID__MAX			4

#define NPROTO_SEMAP_ID__DEV_EVT	0
#define NPROTO_SEMAP_ID__MAX		1

#define NPROTO__MAX_DEV_EVENTS		32

#define NPROTO__MAX_IO_REQUESTS		4

#define NPROTO_EVENT__DISABLED		0
#define NPROTO_EVENT__ENABLED		1
#define NPROTO_EVENT__DEV_ARRIVAL	2
#define NPROTO_EVENT__DEV_REMOVAL	3

#define NPROTO_MEM_SIZE				1024

#pragma pack(push)
#pragma pack(1)

extern "C"
{
#define NPROTO_REQUEST_DATA_SIZE		64
#define NPROTO_REQUEST_DEF_RESULT		0xFFFFFFFF

	typedef struct _NPROTO_REQUEST {
		ULONG	Pid;
		ULONG	Tid;
		ULONG	Result;
		ULONG	Length;
		UCHAR	Data[NPROTO_REQUEST_DATA_SIZE];
	} NPROTO_REQUEST, *PNPROTO_REQUEST;

	typedef struct _NPROTO_EVENT_SLOT {
		ULONG	Pid;
		ULONG	Status;
	} NPROTO_EVENT_SLOT, *PNPROTO_EVENT_SLOT;

	typedef struct _NPROTO_EVENT {
		ULONG				InitialStatus;
		ULONG				Counter;
		NPROTO_EVENT_SLOT	Slots[NPROTO__MAX_DEV_EVENTS];
	} NPROTO_EVENT, *PNPROTO_EVENT;

#define NPROTO_SVC_STATUS__DISABLED		0
#define NPROTO_SVC_STATUS__ENABLED		1

	typedef struct _NPROTO_DATA {
		ULONG			SvcStatus;
		NPROTO_EVENT	Event;
		NPROTO_REQUEST	Write;
		NPROTO_REQUEST	Read;
		UCHAR			Pad[184];
	} NPROTO_DATA, *PNPROTO_DATA;
}

#pragma pack(pop)

#endif  // End of NPORT_SVC_PROTO_DEFINED;

class NSecurityDescriptor
{
public:
	NSecurityDescriptor();
	~NSecurityDescriptor();

	bool					Build(LPCTSTR szDescriptor);
	void					Free();
	PSECURITY_DESCRIPTOR	GetPtr();

private:
	LPCTSTR					m_szDefDesc;
	PSECURITY_DESCRIPTOR	m_pSecDesc;
};

class NSynch
{
public:
	NSynch();
	virtual ~NSynch() = 0;

	DWORD	GetError();

	bool	AcquireDevEventLock(DWORD dwMSecTimeOut, LPDWORD pdwResult);
	bool	ReleaseDevEventLock();

	bool	AcquireReadLock(DWORD dwMSecTimeOut, LPDWORD pdwResult);
	bool	ReleaseReadLock();
	
	bool	AcquireWriteLock(DWORD dwMSecTimeOut, LPDWORD pdwResult);
	bool	ReleaseWriteLock();

	bool	AcquireReadWriteLock(DWORD dwMSecTimeOut, LPDWORD pdwReadResult, LPDWORD pdwWriteResult);
	bool	ReleaseReadWriteLock();

	bool	SetReadRequest();
	bool	ResetReadRequest();
	bool	SetReadComplete();
	bool	ResetReadComplete();

	bool	SetWriteRequest();
	bool	ResetWriteRequest();
	bool	SetWriteCompelete();
	bool	ResetWriteComplete();

protected:
	const TCHAR *	m_szMutexNamePrefix;
	const TCHAR *	m_szEventNamePrefix;
	const TCHAR *	m_szSemNamePrefix;

	HANDLE			m_hMutexes[NPROTO_MTX_ID__MAX];
	HANDLE			m_hEvents[NPROTO_EVT_ID__MAX];
	HANDLE			m_hSemaps[NPROTO_SEMAP_ID__MAX];

	DWORD			m_dwError;

	void			_Destroy();
};

class ClientSynch: public NSynch
{
private:
	bool _OpenMutex(DWORD dwMutexId);
	bool _OpenEvent(DWORD dwEventId);
	bool _OpenSemap(DWORD dwSemapId);

public:
	ClientSynch();
	~ClientSynch();

	bool Open();
	void Close();

	bool AcquireDevEventLock();
	bool ReleaseDevEvent();
	DWORD WaitForDevEvent(DWORD dwMSecTimeOut);

	bool RequestReadAndWait(DWORD dwMSecTimeOut, LPDWORD pdwWaitStat);

	bool RequestWriteAndWait(DWORD dwMSecTimeOut, LPDWORD pdwWaitStat);
};

class ServerSynch: public NSynch
{
private:
	NSecurityDescriptor		m_MtxSecDesc;
	NSecurityDescriptor		m_EvtSecDesc;
	NSecurityDescriptor		m_SemSecDesc;

	bool _CreateMutex(DWORD dwMutexId);
	bool _CreateEvent(DWORD dwEventId);
	bool _CreateSemaphore(DWORD dwSemapId);

public:
	ServerSynch();
	~ServerSynch();

	enum Request {
		Req_None,
		Req_Read,
		Req_Write
	};

	bool Create();
	void Destroy();

	bool	AcquireLock();
	bool	ReleaseLock();

	bool ReleaseDevEvent(LONG lNumberOfEvents, PLONG pPrevNumberOfEvents);

	bool WaitForReadRequest(DWORD dwMSecTimeOut);
	bool SetReadCompletion();

	bool WaitForWriteRequest(DWORD dwMSecTimeOut);
	bool SetWriteCompletion();

	bool WaitForReadWriteRequest(DWORD dwMSecTimeOut, ServerSynch::Request & RequestResult);	
};

class NFileMapping
{
public:
	NFileMapping();
	virtual ~NFileMapping() = 0;

	DWORD	GetSize();
	DWORD	GetError();

	DWORD	GetEventError();
	DWORD	GetReadError();
	DWORD	GetWriteError();

protected:
	const DWORD		m_dwSize;
	const TCHAR *	m_szMapName;
	HANDLE			m_hMapFile;
	LPVOID			m_pMem;
	DWORD			m_dwEventErr;
	DWORD			m_dwReadErr;
	DWORD			m_dwWriteErr;
	DWORD			m_dwErr;

	PNPROTO_DATA	_GetProtoDataPtr();

	void _Destroy();
};

class ClientFileMapping: public NFileMapping
{
private:
	bool _Open();

public:
	ClientFileMapping();
	~ClientFileMapping();

	bool Open();
	void Close();

	bool RequestDevEvent(ULONG & uIndex, ULONG & uInitialValue);
	bool GetDevEventStatus(ULONG uIndex, ULONG & ulEventStatus, BOOL fEnableStatus);
	bool GetDevEventStatus(ULONG uIndex, ULONG & ulEventStatus);
	bool DisableDevEvent(ULONG ulIndex);

	bool RequestRead(ULONG ulRequestedLength);
	bool Read(PUCHAR pBuffer, ULONG uBufferLength, ULONG & uRead);
	bool GetReadResult(ULONG & ulResult);
	bool GetReadResult(PULONG pulResult);

	bool Write(PUCHAR pData, ULONG uDataLength);
	bool GetWriteResult(ULONG & ulResult);
	bool GetWriteResult(PULONG pulResult);
};

class ServerFileMapping: public NFileMapping
{
private:
	NSecurityDescriptor		m_SecDesc;
	
	bool _Create();

public:
	ServerFileMapping();
	~ServerFileMapping();

	bool Create();
	void Destroy();

	bool Initialize(ULONG ulIntialEvent);
	bool Uninitialize();
	bool SetServiceStatus(ULONG ulSvcStatus);

	bool SetDevEventsStatus(ULONG ulEventStatus, ULONG & uCount);

	bool SetReadResult(DWORD dwErrorCode, ULONG ulLengthResult);
	PUCHAR GetReadBuffer(ULONG & ulRequestedLength);
	bool GetReadRequestLength(ULONG & ulRequestedLength);
	bool GetReadRequestor(ULONG & ulPid, ULONG & ulTid);

	bool SetWriteResult(DWORD dwErrorCode, ULONG ulLengthResult);
	PUCHAR GetWriteBuffer(ULONG & ulRequestedLength);
	bool GetWriteRequestor(ULONG & ulPid, ULONG & ulTid);

	bool CopyToReadBuffer(PUCHAR pData, ULONG ulDataLength);
	bool CopyFromWriteBuffer(PUCHAR pBuffer, ULONG ulBufferLength, ULONG & ulCopiedLength);
};
