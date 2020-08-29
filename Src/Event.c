/*
 * File    : Event.c
 * Remark  : Event implementations. 
 *
 */

#include "NanoOS.h"
#include "Event.h"
#include "Thread.h"
#include "Error.h"

static UINT8_T			sEvents[EVT__MAX_EVENTS];
static UINT32_T			suLastError;

void EvtInitialize()
{
	UINT16_T		u;

	for (u = 0; u < EVT__MAX_EVENTS; u++) {
		sEvents[u] = 0;
	}
	suLastError = 0;
}

/*
BOOL EvtCreate(UINT32_PTR_T puiEventId, PTHREAD pRequestingThread)
{
	UINT32_T		u;

	if (!puiEventId) {
		if (pRequestingThread) {
			pRequestingThread->LastError = ERR__INVALID_PARAMETER;
		} else {
			ThdSetLastError(ERR__INVALID_PARAMETER);
		}
		return FALSE;
	}

	for (u = 0; u < EVT__MAX_EVENTS; u++)
	{
		if ((sEvents[u] & EVT__CREATE_BIT) == 0) {
			sEvents[u] = EVT__CREATE_BIT;
			break;
		}
	}

	if (u == EVT__MAX_EVENTS) {
		if (pRequestingThread) {
			pRequestingThread->LastError = ERR__NO_MORE_EVENT_SLOT;
		} else {
			ThdSetLastError(ERR__NO_MORE_EVENT_SLOT);
		}
		return FALSE;
	}

	*puiEventId = u;

	return TRUE;
}

BOOL EvtClose(UINT32_T uiEventId, PTHREAD pRequestingThread)
{
	if (uiEventId >= EVT__MAX_EVENTS) {
		if (pRequestingThread) {
			pRequestingThread->LastError = ERR__INVALID_EVENT_ID;
		} else {
			ThdSetLastError(ERR__INVALID_EVENT_ID);
		}
		return FALSE;
	}

	sEvents[uiEventId] = 0;

	return TRUE;
}

BOOL EvtSet(UINT32_T uiEventId, PTHREAD pRequestingThread)
{
	if (uiEventId >= EVT__MAX_EVENTS) {
		if (pRequestingThread) {
			pRequestingThread->LastError = ERR__INVALID_EVENT_ID;
		} else {
			ThdSetLastError(ERR__INVALID_EVENT_ID);
		}
		return FALSE;
	}

	if ((sEvents[uiEventId] & EVT__CREATE_BIT) == 0) {
		if (pRequestingThread) {
			pRequestingThread->LastError = ERR__INVALID_EVENT_STATE;
		} else {
			ThdSetLastError(ERR__INVALID_EVENT_STATE);
		}
		return FALSE;
	}

	sEvents[uiEventId] |= EVT__SET_BIT;

	return TRUE;
}

BOOL EvtReset(UINT32_T uiEventId, PTHREAD pRequestingThread)
{
	if (uiEventId >= EVT__MAX_EVENTS) {
		if (pRequestingThread) {
			pRequestingThread->LastError = ERR__INVALID_EVENT_ID;
		} else {
			ThdSetLastError(ERR__INVALID_EVENT_ID);
		}
		return FALSE;
	}

	if ((sEvents[uiEventId] & EVT__CREATE_BIT) == 0) {
		if (pRequestingThread) {
			pRequestingThread->LastError = ERR__INVALID_EVENT_STATE;
		} else {
			ThdSetLastError(ERR__INVALID_EVENT_STATE);
		}
		return FALSE;
	}

	sEvents[uiEventId] &= ~(EVT__SET_BIT);

	return TRUE;
}

BOOL EvtGetState(UINT32_T uiEventId, UINT16_PTR_T puiState, PTHREAD pRequestingThread)
{
	if (uiEventId >= EVT__MAX_EVENTS) {
		if (pRequestingThread) {
			pRequestingThread->LastError = ERR__INVALID_EVENT_ID;
		} else {
			ThdSetLastError(ERR__INVALID_EVENT_ID);
		}
		return FALSE;
	}

	if (!puiState) {
		if (pRequestingThread) {
			pRequestingThread->LastError = ERR__INVALID_PARAMETER;
		} else {
			ThdSetLastError(ERR__INVALID_PARAMETER);
		}
		return FALSE;
	}

	if ((sEvents[uiEventId] & EVT__CREATE_BIT) == 0) {
		if (pRequestingThread) {
			pRequestingThread->LastError = ERR__INVALID_EVENT_STATE;
		} else {
			ThdSetLastError(ERR__INVALID_EVENT_STATE);
		}
		return FALSE;
	}

	*puiState = sEvents[uiEventId];

	return TRUE;
}
*/

BOOL EvtCreate(UINT32_PTR_T puiEventId)
{
	UINT32_T		u;

	if (!puiEventId) {
		suLastError = ERR__INVALID_PARAMETER;
		return FALSE;
	}

	for (u = 0; u < EVT__MAX_EVENTS; u++)
	{
		if ((sEvents[u] & EVT__CREATE_BIT) == 0) {
			sEvents[u] = EVT__CREATE_BIT;
			break;
		}
	}

	if (u == EVT__MAX_EVENTS) {
		suLastError = ERR__NO_MORE_EVENT_SLOT;
		return FALSE;
	}

	*puiEventId = u;

	return TRUE;
}

BOOL EvtClose(UINT32_T uiEventId)
{
	if (uiEventId >= EVT__MAX_EVENTS) {
		suLastError = ERR__INVALID_EVENT_ID;
		return FALSE;
	}

	sEvents[uiEventId] = 0;

	return TRUE;
}

BOOL EvtSet(UINT32_T uiEventId)
{
	if (uiEventId >= EVT__MAX_EVENTS) {
		suLastError = ERR__INVALID_EVENT_ID;
		return FALSE;
	}

	if ((sEvents[uiEventId] & EVT__CREATE_BIT) == 0) {
		suLastError = ERR__INVALID_EVENT_STATE;
		return FALSE;
	}

	sEvents[uiEventId] |= EVT__SET_BIT;

	return TRUE;
}

BOOL EvtReset(UINT32_T uiEventId)
{
	if (uiEventId >= EVT__MAX_EVENTS) {
		suLastError = ERR__INVALID_EVENT_ID;
		return FALSE;
	}

	if ((sEvents[uiEventId] & EVT__CREATE_BIT) == 0) {
		suLastError = ERR__INVALID_EVENT_STATE;
		return FALSE;
	}

	sEvents[uiEventId] &= ~(EVT__SET_BIT);

	return TRUE;
}

BOOL EvtGetState(UINT32_T uiEventId, UINT8_PTR_T puiState)
{
	if (uiEventId >= EVT__MAX_EVENTS) {
		suLastError = ERR__INVALID_EVENT_ID;
		return FALSE;
	}

	if (!puiState) {
		suLastError = ERR__INVALID_PARAMETER;
		return FALSE;
	}

	if ((sEvents[uiEventId] & EVT__CREATE_BIT) == 0) {
		suLastError = ERR__INVALID_EVENT_STATE;
		return FALSE;
	}

	*puiState = sEvents[uiEventId];

	return TRUE;
}

UINT32_T EvtGetError()
{
	return suLastError;
}