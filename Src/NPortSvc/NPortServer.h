/*
 * File    : NPortServer.h
 * Remark  : Server implementation of the service protocol.
 *
 */

#pragma once
#include "Protocol.h"

extern "C" {
	typedef struct _NSTATE_LOCK
	{
		volatile LONG	Lock;
		volatile LONG	Counter;
	} NSTATE_LOCK, *PNSTATE_LOCK;
}

class NStateLock
{
public:
	NStateLock();
	~NStateLock();

	bool Initialize();
	void Set();
	void Set(bool bResetCounter);
	bool Acquire();
	void Release();
	void Reset();
	void ResetAndWait();
	LONG GetAcquireCounter();
	LONG GetReleaseCounter();

	static bool Initialize(PNSTATE_LOCK pLock);
	static void Set(PNSTATE_LOCK pLock);
	static bool Acquire(PNSTATE_LOCK pLock);
	static void Release(PNSTATE_LOCK pLock);
	static void ResetAndWait(PNSTATE_LOCK pLock);

private:
	volatile LONG m_lLock;
	volatile LONG m_lCounter;
	volatile LONG m_lAcqCnt;
	volatile LONG m_lRelCnt;
};

class NPortServer
{
public:
	NPortServer();
	~NPortServer();

	const TCHAR *	GetDeviceName();
	
	DWORD			GetError();
	void			ResetError();

	bool			SetDeviceEvent(bool fArrival);
	bool			Start();
	bool			Stop();
	bool			Suspend();
	bool			Resume();

private:
	static volatile bool	m_bStopAcceptIo;
	volatile HANDLE			m_hAcceptIoTh;
	volatile DWORD			m_dwErr;

	const TCHAR *			m_szPortName;

	volatile bool			m_bConnected;
	ULONG					m_ulEventIndex;
	volatile bool			m_bIsNPortOpened;

	ServerSynch				m_Synch;
	ServerFileMapping		m_MapFile;
	NStateLock				m_StateLock;

	DWORD					m_dwReadRes;
	DWORD					m_dwReadLen;
	UCHAR					m_ucReadBuf[NPROTO_REQUEST_DATA_SIZE];

	DWORD					m_dwWriteRes;
	DWORD					m_dwWriteLen;
	UCHAR					m_ucWriteBuf[NPROTO_REQUEST_DATA_SIZE];

	static DWORD CALLBACK	_AcceptIo(LPVOID pDat);
	static DWORD CALLBACK	_OpenNPort(LPVOID pContex);
	static DWORD CALLBACK	_ReadNPort(LPVOID pContext);
	static DWORD CALLBACK	_WriteNPort(LPVOID pContext);

	void _TerminateThread(PLONG pHandle, DWORD dwMSecTimeOut);
	bool _Stop();
	bool _IsPortExist();

	bool _HandleReadRequest();
	bool _HandleWriteRequest();

	HANDLE					m_hNPort;
	void _CloseNPort();
};
