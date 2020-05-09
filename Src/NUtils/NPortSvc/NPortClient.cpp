/*
 * File    : NPortClient.cpp
 * Remark  : NPortClient class implementation.
 *
 */

#include "NPortClient.h"
#include "..\NTerminal\Debug.h"

NPortClient::NPortClient():
	m_hDevEventTh(NULL),
	m_bStopDevEventReq(true),
	m_bDevConnected(false),
	m_bActiveService(false),
	m_uEventIndex(0)
{
}

NPortClient::~NPortClient()
{
	_Close();
}

void NPortClient::_TerminateThread(PLONG pHandle, DWORD dwMSecTimeOut)
{
	DWORD	dwRes = 0;
	HANDLE	hTh = NULL;

	if (!pHandle) return;
	hTh = InterlockedExchangePointer(pHandle, NULL);
	if (hTh) {
		dwRes = WaitForSingleObject(hTh, dwMSecTimeOut);
		if (dwRes == WAIT_TIMEOUT) {
			TerminateThread(hTh, ERROR_OPERATION_ABORTED);
		}
		CloseHandle(hTh);
	}
}

void NPortClient::_Close()
{
	m_bStopDevEventReq = true;
	m_bDevConnected = false;
	m_bActiveService = false;

	_TerminateThread((PLONG) &m_hDevEventTh, 2000);

	m_Synch.AcquireDevEventLock();
	m_MapFile.DisableDevEvent(m_uEventIndex);
	m_Synch.ReleaseDevEventLock();

	m_MapFile.Close();
	m_Synch.Close();
}

bool NPortClient::Open()
{
	bool	bRes = false;
	ULONG	ulEventInitValue = 0;

	if (m_hDevEventTh) return true;

	m_bActiveService = false;

	do {
		bRes = m_Synch.Open();
		if (!bRes) {
			m_dwErr = m_Synch.GetError();
			break;
		}

		bRes = m_MapFile.Open();
		if (!bRes) {
			m_dwErr = m_MapFile.GetError();
			break;
		}

		bRes = m_Synch.AcquireDevEventLock();
		if (bRes) {
			bRes = m_MapFile.RequestDevEvent(m_uEventIndex, ulEventInitValue);
			if (!bRes) m_dwErr = m_MapFile.GetError();
			m_Synch.ReleaseDevEventLock();
		} else {
			m_dwErr = m_Synch.GetError();
		}

		if (!bRes) break;

		m_bStopDevEventReq = false;
		m_hDevEventTh = CreateThread(
							NULL,
							0,
							reinterpret_cast<LPTHREAD_START_ROUTINE>(&NPortClient::_DoDevEventReq),
							reinterpret_cast<LPVOID>(this),
							0,
							NULL
							);
		if (!m_hDevEventTh) {
			m_bStopDevEventReq = true;
			m_dwErr = GetLastError();
			break;
		}

		m_dwErr = ERROR_SUCCESS;
	} while (FALSE);

	if (m_dwErr != ERROR_SUCCESS)
	{
		_Close();
	}
	else
	{
		switch (ulEventInitValue)
		{
		case NPROTO_EVENT__DEV_ARRIVAL:
			m_bDevConnected = true;
			ConnectionChanged(true);
			break;

		case NPROTO_EVENT__DEV_REMOVAL:
			m_bDevConnected = false;
			ConnectionChanged(false);
			break;

		default:
			DBG_PRINTF(_T("%s: Warning: Unexpected initial value! (%d)\r\n"), FUNCT_NAME_STR, ulEventInitValue);
			break;
		}

		m_bActiveService = true;
	}

	return ((m_dwErr == ERROR_SUCCESS) ? true : false);
}

void NPortClient::Close()
{
	_Close();
}

DWORD CALLBACK NPortClient::_DoDevEventReq(LPVOID pDat)
{
	NPortClient *	pThis = reinterpret_cast<NPortClient *>(pDat);
	DWORD			dwRes = 0;
	HANDLE			hTh = NULL;

	if (!pThis) {
		dwRes = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	while (!pThis->m_bStopDevEventReq)
	{
		dwRes = pThis->m_Synch.WaitForDevEvent(1000);
		if (pThis->m_bStopDevEventReq) break;

		if (dwRes == WAIT_FAILED) {
			pThis->m_dwErr = pThis->m_Synch.GetError();
			pThis->m_bStopDevEventReq = true;
			break;
		}

		switch (dwRes)
		{
		case WAIT_OBJECT_0:
			pThis->_HandleDevEvent();
			break;

		case WAIT_TIMEOUT:
			break;

		default:
			break;
		}
	}

Cleanup:
	if (pThis) {
		hTh = InterlockedExchangePointer((PLONG) &(pThis->m_hDevEventTh), NULL);
		if (hTh) {
			CloseHandle(hTh);
		}
	}

	ExitThread(dwRes);
	return dwRes;
}

bool NPortClient::_HandleDevEvent()
{
	ULONG	ulStatus = 0;
	bool	bRes = false;

	bRes = m_Synch.AcquireDevEventLock();
	if (bRes) {
		bRes = m_MapFile.GetDevEventStatus(m_uEventIndex, ulStatus);
		m_Synch.ReleaseDevEventLock();
		if (bRes) {
			switch (ulStatus)
			{
			case NPROTO_EVENT__DEV_ARRIVAL:
				DBG_PRINTF(_T("%s: Device arrival.\r\n"), FUNCT_NAME_STR);
				m_bDevConnected = true;
				ConnectionChanged(true);
				break;

			case NPROTO_EVENT__DEV_REMOVAL:
				DBG_PRINTF(_T("%s: Device removal.\r\n"), FUNCT_NAME_STR);
				m_bDevConnected = false;
				ConnectionChanged(false);
				break;

			case NPROTO_EVENT__ENABLED:
				DBG_PRINTF(_T("%s: Already handle dev. event.\r\n"), FUNCT_NAME_STR);
				m_Synch.ReleaseDevEvent();
				break;

			default:
				DBG_PRINTF(_T("%s: Warning: Unexpected event: %d!\r\n"), FUNCT_NAME_STR, ulStatus);
				break;
			}
		} else {
			m_dwErr = m_MapFile.GetError();
			if (m_dwErr == ERROR_SERVICE_NOT_ACTIVE) 
				m_bActiveService = false;
		}
	} else {
		m_dwErr = m_Synch.GetError();
	}

	return bRes;
}

bool NPortClient::_Read(PUCHAR pBuffer, ULONG ulBufferLength, PULONG pReadLength)
{
	bool	bRes = false;
	ULONG	ulReadLen = 0;
	ULONG	ulResult = 0;
	DWORD	dwRes = 0;

	//DBG_PRINTF(_T("%s: PID=%d\r\n"), FUNCT_NAME_STR, GetCurrentProcessId());

	do {
		bRes = m_MapFile.RequestRead(ulBufferLength);
		if (!bRes) {
			m_dwErr = m_MapFile.GetReadError();
			if (m_dwErr == ERROR_SERVICE_NOT_ACTIVE) 
				m_bActiveService = false;
			break;
		}

		bRes = m_Synch.RequestReadAndWait(INFINITE, NULL);
		if (!bRes) {
			m_dwErr = m_Synch.GetError();
			break;
		}

		bRes = m_MapFile.GetReadResult(&ulResult);
		if (!bRes) {
			m_dwErr = m_MapFile.GetReadError();
			if (m_dwErr == ERROR_SERVICE_NOT_ACTIVE) 
				m_bActiveService = false;
			break;
		}

		if (ulResult != ERROR_SUCCESS) {
			if (ulResult == NPROTO_REQUEST_DEF_RESULT) {
				m_dwErr = ERROR_CAN_NOT_COMPLETE;
			} else {
				m_dwErr = ulResult;
			}
			bRes = false;
			break;
		}

		bRes = m_MapFile.Read(pBuffer, ulBufferLength, ulReadLen);
		if (!bRes) {
			m_dwErr = m_MapFile.GetReadError();
			if (m_dwErr == ERROR_SERVICE_NOT_ACTIVE) 
				m_bActiveService = false;
			break;
		}

		if (pReadLength) {
			*pReadLength = ulReadLen;
		}

		m_dwErr = ERROR_SUCCESS;
	} while (FALSE);

	return bRes;
}

bool NPortClient::_Write(const PUCHAR pData, ULONG ulDataLength)
{
	bool	bRes = false;
	ULONG	ulResult = 0;
	DWORD	dwRes = 0;

	//DBG_PRINTF(_T("%s: PID=%d\r\n"), FUNCT_NAME_STR, GetCurrentProcessId());

	do {
		bRes = m_MapFile.Write(const_cast<PUCHAR>(pData), ulDataLength);
		if (!bRes) {
			m_dwErr = m_MapFile.GetWriteError();
			if (m_dwErr == ERROR_SERVICE_NOT_ACTIVE) 
				m_bActiveService = false;
			break;
		}

		bRes = m_Synch.RequestWriteAndWait(INFINITE, NULL);
		if (!bRes) {
			m_dwErr = m_Synch.GetError();
			break;
		}

		bRes = m_MapFile.GetWriteResult(&ulResult);
		if (!bRes) {
			m_dwErr = m_MapFile.GetWriteError();
			if (m_dwErr == ERROR_SERVICE_NOT_ACTIVE) 
				m_bActiveService = false;
			break;
		}

		if (ulResult != ERROR_SUCCESS) {
			if (ulResult == NPROTO_REQUEST_DEF_RESULT) {
				m_dwErr = ERROR_CAN_NOT_COMPLETE;
			} else {
				m_dwErr = ulResult;
			}
			bRes = false;
			break;
		}

		m_dwErr = ERROR_SUCCESS;
	} while (FALSE);

	return bRes;
}

DWORD NPortClient::GetError()
{
	return m_dwErr;
}

bool NPortClient::Read(unsigned char *pBuffer, unsigned int uiBufferLength, unsigned int &uiRead)
{
	if (!m_bActiveService) {
		m_dwErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if ((!pBuffer) || 
		(uiBufferLength == 0))
	{
		m_dwErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	if (!m_bDevConnected) {
		m_dwErr = ERROR_DEVICE_NOT_CONNECTED;
		return false;
	}

	bool	bRes = false;
	DWORD	dwLockStat = 0;

	bRes = m_Synch.AcquireReadLock(INFINITE, &dwLockStat);
	if (!bRes) {
		m_dwErr = m_Synch.GetError();
		return false;
	}

	if (dwLockStat == WAIT_ABANDONED) {
		DBG_PRINTF(_T("%s: Another thread abandoned the lock, wait a moment...\r\n"), FUNCT_NAME_STR);
		Sleep(500);

		m_Synch.ResetReadRequest();
		m_Synch.ResetReadComplete();
	}

	ULONG	ulMaxDevBuf = (NPROTO_REQUEST_DATA_SIZE - 2);
	ULONG	ulRemaining = uiBufferLength;
	ULONG	ulDataRead = 0;
	PUCHAR	pBuff = pBuffer;

	uiRead = 0;

	do {
		if (ulRemaining > ulMaxDevBuf) {
			ulDataRead = ulMaxDevBuf;
		} else {
			ulDataRead = ulRemaining;
		}

		bRes = _Read(pBuff, ulDataRead, &ulDataRead);

		if (bRes) {
			pBuff += ulDataRead;
			uiRead += ulDataRead;
			if (ulRemaining > 0) {
				if (ulRemaining >= ulDataRead) {
					ulRemaining -= ulDataRead;
				}
			}
		} else {
			break;
		}
	} while (ulRemaining > 0);

	m_Synch.ReleaseReadLock();

	return bRes;
}

bool NPortClient::Write(const unsigned char *pData, unsigned int uiDataLength)
{
	if (!m_bActiveService) {
		m_dwErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if ((!pData) || 
		(uiDataLength == 0))
	{
		m_dwErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	if (!m_bDevConnected) {
		m_dwErr = ERROR_DEVICE_NOT_CONNECTED;
		return false;
	}

	bool	bRes = false;
	DWORD	dwLockStat = 0;

	bRes = m_Synch.AcquireWriteLock(INFINITE, &dwLockStat);
	if (!bRes) {
		m_dwErr = m_Synch.GetError();
		return false;
	}

	if (dwLockStat == WAIT_ABANDONED) {
		DBG_PRINTF(_T("%s: Another thread abandoned the lock, wait a moment...\r\n"), FUNCT_NAME_STR);
		Sleep(500);

		m_Synch.ResetWriteRequest();
		m_Synch.ResetWriteComplete();
	}

	ULONG	ulWriteData = 0;
	ULONG	ulRemaining = uiDataLength;
	PUCHAR	pBuf = const_cast<PUCHAR>(pData);

	do {
		if (ulRemaining > NPROTO_REQUEST_DATA_SIZE) {
			ulWriteData = NPROTO_REQUEST_DATA_SIZE;
		} else {
			ulWriteData = ulRemaining;
		}

		bRes = _Write(const_cast<const PUCHAR>(pBuf), ulWriteData);

		if (bRes) {
			pBuf += ulWriteData;
			if (ulRemaining >= ulWriteData) {
				ulRemaining -= ulWriteData;
			}
		} else {
			break;
		}
	} while (ulRemaining > 0);

	m_Synch.ReleaseWriteLock();

	return bRes;
}

bool NPortClient::WriteThenRead(
								const unsigned char *pWriteData, 
								unsigned int uiWriteDataLength, 
								unsigned char *pReadBuffer, 
								unsigned int uiReadBufferLength, 
								unsigned int &uiReadLength
								)
{
	if (!m_bActiveService) {
		m_dwErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if ((!pWriteData) || 
		(uiWriteDataLength == 0) ||
		(!pReadBuffer) ||
		(uiReadBufferLength == 0))
	{
		m_dwErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	if (!m_bDevConnected) {
		m_dwErr = ERROR_DEVICE_NOT_CONNECTED;
		return false;
	}

	bool	bRes = false;
	DWORD	dwRdLockStat;
	DWORD	dwWrLockStat;

	bRes = m_Synch.AcquireReadWriteLock(INFINITE, &dwRdLockStat, &dwWrLockStat);
	if (!bRes) {
		m_dwErr = m_Synch.GetError();
		return false;
	}

	if ((dwRdLockStat == WAIT_ABANDONED) ||
		(dwWrLockStat == WAIT_ABANDONED))
	{
		DBG_PRINTF(_T("%s: Another thread abandoned the lock, wait a moment...\r\n"), FUNCT_NAME_STR);
		Sleep(500);

		m_Synch.ResetReadRequest();
		m_Synch.ResetReadComplete();
		m_Synch.ResetWriteRequest();
		m_Synch.ResetWriteComplete();
	}

	//DBG_PRINTF(_T("%s: [%d] Begin\r\n"), FUNCT_NAME_STR, GetCurrentProcessId());

	ULONG	ulWriteData = 0;
	ULONG	ulRemaining = uiWriteDataLength;
	PUCHAR	pBuf = const_cast<PUCHAR>(pWriteData);

	do {
		if (ulRemaining > NPROTO_REQUEST_DATA_SIZE) {
			ulWriteData = NPROTO_REQUEST_DATA_SIZE;
		} else {
			ulWriteData = ulRemaining;
		}

		bRes = _Write(const_cast<const PUCHAR>(pBuf), ulWriteData);

		if (bRes) {
			pBuf += ulWriteData;
			if (ulRemaining >= ulWriteData) {
				ulRemaining -= ulWriteData;
			}
		} else {
			break;
		}
	} while (ulRemaining > 0);

	if (bRes) 
	{
		ULONG	ulDataRead = 0;

		ulRemaining		= uiReadBufferLength;
		pBuf			= pReadBuffer;
		uiReadLength	= 0;

		do {
			if (ulRemaining > (NPROTO_REQUEST_DATA_SIZE - 2)) {
				ulDataRead = (NPROTO_REQUEST_DATA_SIZE - 2);
			} else {
				ulDataRead = ulRemaining;
			}

			bRes = _Read(pBuf, ulDataRead, &ulDataRead);

			if (bRes) {
				pBuf += ulDataRead;
				uiReadLength += ulDataRead;
				if (ulRemaining > 0) {
					if (ulRemaining >= ulDataRead) {
						ulRemaining -= ulDataRead;
					}
				}
			} else {
				break;
			}
		} while (ulRemaining > 0);
	}

	//DBG_PRINTF(_T("%s: [%d] End\r\n"), FUNCT_NAME_STR, GetCurrentProcessId());

	m_Synch.ReleaseReadWriteLock();

	return bRes;
}

bool NPortClient::ReadThenWrite(
								unsigned char *pReadBuffer, 
								unsigned int uiReadBufferLength, 
								unsigned int &uiReadLength, 
								const unsigned char *pWriteData, 
								unsigned int uiWriteDataLength
								)
{
	if (!m_bActiveService) {
		m_dwErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if ((!pWriteData) || 
		(uiWriteDataLength == 0) ||
		(!pReadBuffer) ||
		(uiReadBufferLength == 0))
	{
		m_dwErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	if (!m_bDevConnected) {
		m_dwErr = ERROR_DEVICE_NOT_CONNECTED;
		return false;
	}

	bool	bRes = false;
	DWORD	dwRdLockStat, dwWrLockStat;

	bRes = m_Synch.AcquireReadWriteLock(INFINITE, &dwRdLockStat, &dwWrLockStat);
	if (!bRes) {
		m_dwErr = m_Synch.GetError();
		return false;
	}
	if ((dwRdLockStat == WAIT_ABANDONED) ||
		(dwWrLockStat == WAIT_ABANDONED))
	{
		DBG_PRINTF(_T("%s: Another thread abandoned the lock, wait a moment...\r\n"), FUNCT_NAME_STR);
		Sleep(500);

		m_Synch.ResetReadRequest();
		m_Synch.ResetReadComplete();
		m_Synch.ResetWriteRequest();
		m_Synch.ResetWriteComplete();
	}

	ULONG	ulRemaining = uiReadBufferLength;
	ULONG	ulDataRead = 0;
	PUCHAR	pBuff = pReadBuffer;

	uiReadLength = 0;

	do {
		if (ulRemaining > (NPROTO_REQUEST_DATA_SIZE - 2)) {
			ulDataRead = (NPROTO_REQUEST_DATA_SIZE - 2);
		} else {
			ulDataRead = ulRemaining;
		}

		bRes = _Read(pBuff, ulDataRead, &ulDataRead);

		if (bRes) {
			pBuff += ulDataRead;
			uiReadBufferLength += ulDataRead;
			if (ulRemaining > 0) {
				if (ulRemaining >= ulDataRead) {
					ulRemaining -= ulDataRead;
				}
			}
		} else {
			break;
		}
	} while (ulRemaining > 0);

	if (bRes)
	{
		ULONG	ulWriteData = 0;

		ulRemaining	= uiWriteDataLength;
		pBuff		= const_cast<PUCHAR>(pWriteData);

		do {
			if (ulRemaining > NPROTO_REQUEST_DATA_SIZE) {
				ulWriteData = NPROTO_REQUEST_DATA_SIZE;
			} else {
				ulWriteData = ulRemaining;
			}

			bRes = _Write(const_cast<const PUCHAR>(pBuff), ulWriteData);

			if (bRes) {
				pBuff += ulWriteData;
				if (ulRemaining >= ulWriteData) {
					ulRemaining -= ulWriteData;
				}
			} else {
				break;
			}
		} while (ulRemaining > 0);
	}

	m_Synch.ReleaseReadWriteLock();

	return bRes;
}

bool NPortClient::WriteThenRead(
								const unsigned char *pWriteData, 
								unsigned int uiWriteDataLength, 
								unsigned char *pReadBuffer, 
								unsigned int uiReadBufferLength, 
								unsigned int &uiReadLength, 
								unsigned long ulMSecDelay
								)
{
	if (!m_bActiveService) {
		m_dwErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if ((!pWriteData) || 
		(uiWriteDataLength == 0) ||
		(!pReadBuffer) ||
		(uiReadBufferLength == 0))
	{
		m_dwErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	if (!m_bDevConnected) {
		m_dwErr = ERROR_DEVICE_NOT_CONNECTED;
		return false;
	}

	bool	bRes = false;
	DWORD	dwRdLockStat;
	DWORD	dwWrLockStat;

	bRes = m_Synch.AcquireReadWriteLock(INFINITE, &dwRdLockStat, &dwWrLockStat);
	if (!bRes) {
		m_dwErr = m_Synch.GetError();
		return false;
	}

	if ((dwRdLockStat == WAIT_ABANDONED) ||
		(dwWrLockStat == WAIT_ABANDONED))
	{
		DBG_PRINTF(_T("%s: Another thread abandoned the lock, wait a moment...\r\n"), FUNCT_NAME_STR);
		Sleep(500);

		m_Synch.ResetReadRequest();
		m_Synch.ResetReadComplete();
		m_Synch.ResetWriteRequest();
		m_Synch.ResetWriteComplete();
	}

	ULONG	ulWriteData = 0;
	ULONG	ulRemaining = uiWriteDataLength;
	PUCHAR	pBuf = const_cast<PUCHAR>(pWriteData);

	do {
		if (ulRemaining > NPROTO_REQUEST_DATA_SIZE) {
			ulWriteData = NPROTO_REQUEST_DATA_SIZE;
		} else {
			ulWriteData = ulRemaining;
		}

		bRes = _Write(const_cast<const PUCHAR>(pBuf), ulWriteData);

		if (bRes) {
			pBuf += ulWriteData;
			if (ulRemaining >= ulWriteData) {
				ulRemaining -= ulWriteData;
			}
		} else {
			break;
		}
	} while (ulRemaining > 0);

	if (bRes) 
	{
		Sleep(ulMSecDelay);

		ULONG	ulDataRead = 0;

		ulRemaining		= uiReadBufferLength;
		pBuf			= pReadBuffer;
		uiReadLength	= 0;

		do {
			if (ulRemaining > (NPROTO_REQUEST_DATA_SIZE - 2)) {
				ulDataRead = (NPROTO_REQUEST_DATA_SIZE - 2);
			} else {
				ulDataRead = ulRemaining;
			}

			bRes = _Read(pBuf, ulDataRead, &ulDataRead);

			if (bRes) {
				pBuf += ulDataRead;
				uiReadLength += ulDataRead;
				if (ulRemaining > 0) {
					if (ulRemaining >= ulDataRead) {
						ulRemaining -= ulDataRead;
					}
				}
			} else {
				break;
			}
		} while (ulRemaining > 0);
	}

	m_Synch.ReleaseReadWriteLock();

	return bRes;
}

bool NPortClient::ReadThenWrite(
								unsigned char *pReadBuffer, 
								unsigned int uiReadBufferLength, 
								unsigned int &uiReadLength, 
								const unsigned char *pWriteData, 
								unsigned int uiWriteDataLength, 
								unsigned long ulMSecDelay
								)
{
	if (!m_bActiveService) {
		m_dwErr = ERROR_RESOURCE_NOT_AVAILABLE;
		return false;
	}

	if ((!pWriteData) || 
		(uiWriteDataLength == 0) ||
		(!pReadBuffer) ||
		(uiReadBufferLength == 0))
	{
		m_dwErr = ERROR_INVALID_PARAMETER;
		return false;
	}

	if (!m_bDevConnected) {
		m_dwErr = ERROR_DEVICE_NOT_CONNECTED;
		return false;
	}

	bool	bRes = false;
	DWORD	dwRdLockStat, dwWrLockStat;

	bRes = m_Synch.AcquireReadWriteLock(INFINITE, &dwRdLockStat, &dwWrLockStat);
	if (!bRes) {
		m_dwErr = m_Synch.GetError();
		return false;
	}
	if ((dwRdLockStat == WAIT_ABANDONED) ||
		(dwWrLockStat == WAIT_ABANDONED))
	{
		DBG_PRINTF(_T("%s: Another thread abandoned the lock, wait a moment...\r\n"), FUNCT_NAME_STR);
		Sleep(500);

		m_Synch.ResetReadRequest();
		m_Synch.ResetReadComplete();
		m_Synch.ResetWriteRequest();
		m_Synch.ResetWriteComplete();
	}

	ULONG	ulRemaining = uiReadBufferLength;
	ULONG	ulDataRead = 0;
	PUCHAR	pBuff = pReadBuffer;

	uiReadLength = 0;

	do {
		if (ulRemaining > (NPROTO_REQUEST_DATA_SIZE - 2)) {
			ulDataRead = (NPROTO_REQUEST_DATA_SIZE - 2);
		} else {
			ulDataRead = ulRemaining;
		}

		bRes = _Read(pBuff, ulDataRead, &ulDataRead);

		if (bRes) {
			pBuff += ulDataRead;
			uiReadBufferLength += ulDataRead;
			if (ulRemaining > 0) {
				if (ulRemaining >= ulDataRead) {
					ulRemaining -= ulDataRead;
				}
			}
		} else {
			break;
		}
	} while (ulRemaining > 0);

	if (bRes)
	{
		Sleep(ulMSecDelay);

		ULONG	ulWriteData = 0;

		ulRemaining	= uiWriteDataLength;
		pBuff		= const_cast<PUCHAR>(pWriteData);

		do {
			if (ulRemaining > NPROTO_REQUEST_DATA_SIZE) {
				ulWriteData = NPROTO_REQUEST_DATA_SIZE;
			} else {
				ulWriteData = ulRemaining;
			}

			bRes = _Write(const_cast<const PUCHAR>(pBuff), ulWriteData);

			if (bRes) {
				pBuff += ulWriteData;
				if (ulRemaining >= ulWriteData) {
					ulRemaining -= ulWriteData;
				}
			} else {
				break;
			}
		} while (ulRemaining > 0);
	}

	m_Synch.ReleaseReadWriteLock();

	return bRes;
}