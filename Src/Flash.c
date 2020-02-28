/*
 * File    : Exception.c
 * Remark  : Contains flash memory routines.
 *
 */

#include "Sys\Stm\Flash.h"

#include "NanoOS.h"
#include "Error.h"
#include "Address.h"

#define MAX_FLASH_PAGE_NO		127

BOOL FlsUnlock()
{
	if (FLASH_IS_MEMORY_LOCKED()) {
		FLASH_UNLOCK_MEMORY();
		if (FLASH_IS_MEMORY_LOCKED())
		{
			ThdSetLastError(ERR__FAIL_TO_UNLOCK_FLASH);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL FlsLock()
{
	if (FLASH_IS_MEMORY_LOCKED())
		return TRUE;

	FLASH_LOCK_MEMORY();
	if (FLASH_IS_MEMORY_LOCKED() == FALSE) {
		ThdSetLastError(ERR__FAIL_TO_LOCK_FLASH);
		return FALSE;
	}

	return TRUE;
}

BOOL FlsGetAddressFromPageNo(UINT32_T uiPageNo, UINT32_PTR_T *ppRetPageAddress)
{
	if (uiPageNo > MAX_FLASH_PAGE_NO) {
		ThdSetLastError(ERR__INVALID_FLASH_PAGE_NO);
		return FALSE;
	}

	if (!ppRetPageAddress) {
		ThdSetLastError(ERR__INVALID_RETURN_VALUE_POINTER);
		return FALSE;
	}

	*ppRetPageAddress = 
		(UINT32_PTR_T) (FLASH_WRITE_BASE_ADDRESS + (uiPageNo + FLASH_PAGE_OFFSET));

	return TRUE;
}

BOOL FlsGetPageNoFromAddress(UINT32_PTR_T pAddress, UINT32_PTR_T pRetPageNo)
{
	UINT32_T	u;
	UINT32_PTR_T	puLow, puHigh;

	if (!pRetPageNo) {
		ThdSetLastError(ERR__INVALID_RETURN_VALUE_POINTER);
		return FALSE;
	}

	for (u = 0; u <= MAX_FLASH_PAGE_NO; u++)
	{
		puLow = (UINT32_PTR_T) (FLASH_WRITE_BASE_ADDRESS + (u * FLASH_PAGE_OFFSET));
		puHigh = (UINT32_PTR_T) (FLASH_WRITE_BASE_ADDRESS + (((u + 1) * FLASH_PAGE_OFFSET) - 1));
		if ((pAddress >= puLow) && (pAddress <= puHigh)) 
			break;
	}

	if (u == (MAX_FLASH_PAGE_NO + 1)) {
		ThdSetLastError(ERR__NO_FLASH_PAGE_FOUND);
		return FALSE;
	}

	*pRetPageNo = u;

	return TRUE;
}

BOOL FlsErase(UINT32_T uiStartPageNo, UINT32_T uiNumbOfPages)
{
	UINT32_T	u;
	UINT32_T	uiAddr;
	
	if (FLASH_IS_MEMORY_LOCKED()) {
		ThdSetLastError(ERR__INVALID_FLASH_OPERATION_STATE);
		return FALSE;
	}

	if (uiStartPageNo > MAX_FLASH_PAGE_NO) {
		ThdSetLastError(ERR__INVALID_FLASH_PAGE_NO);
		return FALSE;
	}

	FLASH_ENABLE_PAGE_ERASE(TRUE);

	for (u = 0; u < uiNumbOfPages; u++)
	{
		uiAddr = (FLASH_WRITE_BASE_ADDRESS + ((u + uiStartPageNo) * FLASH_PAGE_OFFSET));

		FLASH_WRITE_ADDRESS(uiAddr);
		FLASH_START_ERASE();
		while (FLASH_IS_BUSY());

		if (FLASH_IS_END_OF_OPERATION() == FALSE) {
			ThdSetLastError(ERR__FLASH_OPERATION);
			break;
		} else {
			FLASH_CLEAR_END_OF_OPERATION();
		}
	}

	FLASH_ENABLE_PAGE_ERASE(FALSE);

	return ((u == uiNumbOfPages) ? TRUE : FALSE);
}

BOOL FlsProgram(UINT16_PTR_T pDestination, UINT16_PTR_T pSource, UINT32_T uiNumberOfOperation)
{
	UINT32_T u;
	UINT32_PTR_T	pTest = (UINT32_PTR_T) pDestination;

	if ((pDestination == 0) ||
		(pSource == 0))
	{
		ThdSetLastError(ERR__INVALID_PARAMETER);
		return FALSE;
	}

	if (FLASH_IS_MEMORY_LOCKED()) {
		ThdSetLastError(ERR__INVALID_FLASH_OPERATION_STATE);
		return FALSE;
	}

	if (uiNumberOfOperation == 0) {
		ThdSetLastError(ERR__INVALID_FLASH_NUMBER_OF_OPERATION);
		return FALSE;
	}

	if (FlsGetPageNoFromAddress(pTest, &u) == FALSE) {
		ThdSetLastError(ERR__INVALID_FLASH_PAGE_NO);
		return FALSE;
	}

	FLASH_ENABLE_PROGRAMMING(TRUE);

	for (u = 0; u < uiNumberOfOperation; u++)
	{
		*pDestination = *pSource;
		while (FLASH_IS_BUSY());

		if (FLASH_IS_PROGRAMMING_ERROR() == TRUE) {
			FLASH_CLEAR_PROGRAMMING_ERROR();
			ThdSetLastError(ERR__FLASH_PROGRAMMING);
			break;
		}

		if (FLASH_IS_WRITE_PROTECTION_ERROR() == TRUE) {
			FLASH_CLEAR_WRITE_PROTECTION_ERROR();
			ThdSetLastError(ERR__FLASH_WRITE_PROTECTION);
			break;
		}

		if (FLASH_IS_END_OF_OPERATION() == TRUE) {
			FLASH_CLEAR_END_OF_OPERATION();
		} else {
			ThdSetLastError(ERR__FLASH_OPERATION);
			break;
		}

		pDestination++;
		pSource++;
	}

	FLASH_ENABLE_PROGRAMMING(FALSE);

	return ((u == uiNumberOfOperation) ? TRUE : FALSE);
}

/*
BOOL FlsProgramFromPaddedSource(UINT16_PTR_T pDestination, UINT32_PTR_T pSource, UINT32_T uiNumberOfOperation)
{
	UINT32_T u;
	UINT32_PTR_T	pTest = (UINT32_PTR_T) pDestination;

	if ((pDestination == 0) ||
		(pSource == 0))
	{
		ThdSetLastError(ERR__INVALID_PARAMETER);
		return FALSE;
	}

	if (FLASH_IS_MEMORY_LOCKED()) {
		ThdSetLastError(ERR__INVALID_FLASH_OPERATION_STATE);
		return FALSE;
	}

	if (uiNumberOfOperation == 0) {
		ThdSetLastError(ERR__INVALID_FLASH_NUMBER_OF_OPERATION);
		return FALSE;
	}

	if (FlsGetPageNoFromAddress(pTest, &u) == FALSE) {
		ThdSetLastError(ERR__INVALID_FLASH_PAGE_NO);
		return FALSE;
	}

	FLASH_ENABLE_PROGRAMMING(TRUE);

	for (u = 0; u < uiNumberOfOperation; u++)
	{
		*pDestination = (UINT16_T) (*pSource);
		while (FLASH_IS_BUSY());

		if (FLASH_IS_PROGRAMMING_ERROR() == TRUE) {
			FLASH_CLEAR_PROGRAMMING_ERROR();
			ThdSetLastError(ERR__FLASH_PROGRAMMING);
			break;
		}

		if (FLASH_IS_WRITE_PROTECTION_ERROR() == TRUE) {
			FLASH_CLEAR_WRITE_PROTECTION_ERROR();
			ThdSetLastError(ERR__FLASH_WRITE_PROTECTION);
			break;
		}

		if (FLASH_IS_END_OF_OPERATION() == TRUE) {
			FLASH_CLEAR_END_OF_OPERATION();
		} else {
			ThdSetLastError(ERR__FLASH_OPERATION);
			break;
		}

		pDestination++;
		pSource++;
	}

	FLASH_ENABLE_PROGRAMMING(FALSE);

	return ((u == uiNumberOfOperation) ? TRUE : FALSE);
}
*/