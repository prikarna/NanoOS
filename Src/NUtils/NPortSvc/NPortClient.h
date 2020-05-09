/*
 * File    : NPortClient.h
 * Remark  : Client implementation of the service protocol.
 *
 */

#pragma once

#include "Protocol.h"
#include "..\NTerminal\DkObject.h"

class NPortClient
{
public:
	NPortClient();
	~NPortClient();

	bool Open();
	void Close();
	bool Read(unsigned char * pBuffer, unsigned int uiBufferLength, unsigned int & uiRead);
	bool Write(const unsigned char * pData, unsigned int uiDataLength);
	bool WriteThenRead(
					const unsigned char * pWriteData,
					unsigned int uiWriteDataLength,
					unsigned char * pReadBuffer,
					unsigned int uiReadBufferLength,
					unsigned int & uiReadLength
					);
	bool ReadThenWrite(
					unsigned char * pReadBuffer,
					unsigned int uiReadBufferLength,
					unsigned int & uiReadLength,
					const unsigned char * pWriteData,
					unsigned int uiWriteDataLength
					);
	bool WriteThenRead(
					const unsigned char * pWriteData,
					unsigned int uiWriteDataLength,
					unsigned char * pReadBuffer,
					unsigned int uiReadBufferLength,
					unsigned int & uiReadLength,
					unsigned long ulMSecDelay
					);
	bool ReadThenWrite(
					unsigned char * pReadBuffer,
					unsigned int uiReadBufferLength,
					unsigned int & uiReadLength,
					const unsigned char * pWriteData,
					unsigned int uiWriteDataLength,
					unsigned long ulMSecDelay
					);
	DWORD GetError();

	Event<NPortClient, bool>		ConnectionChanged;

private:
	volatile HANDLE		m_hDevEventTh;
	volatile DWORD		m_dwErr;
	
	ULONG		m_uEventIndex;

	volatile bool		m_bStopDevEventReq;
	volatile bool		m_bDevConnected;
	volatile bool		m_bActiveService;

	ClientFileMapping	m_MapFile;
	ClientSynch			m_Synch;

	static DWORD CALLBACK	_DoDevEventReq(LPVOID pDat);

	void _TerminateThread(PLONG pHandle, DWORD dwMSecTimeOut);
	void _Close();
	bool _HandleDevEvent();
	bool _Read(PUCHAR pBuffer, ULONG ulBufferLength, PULONG pReadLength);
	bool _Write(const PUCHAR pData, ULONG ulDataLength);
};
