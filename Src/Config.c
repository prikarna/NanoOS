/*
 * File    : Config.c
 * Remark  : Configuration implementation in NanoOS, mostly flash operation 
 *           with specific data structure.
 *
 */

#include "Sys\Stm\Rcc.h"
#include "Sys\Stm\Flash.h"

#include "NanoOS.h"
#include "Thread.h"
#include "Console.h"
#include "Config.h"
#include "Address.h"

static CONFIG	sConfig;

__attribute__ ((unused, naked, section(".nanoos_config")))
static void ConfigData()
{
	__asm volatile("NOP");
	__asm volatile("NOP");
	__asm volatile("NOP");
	__asm volatile("NOP");
	__asm volatile("NOP");
	__asm volatile("NOP");
	__asm volatile("NOP");
	__asm volatile("NOP");
}

PCONFIG CfgGet()
{
	return &sConfig;
}

BOOL CfgLoad()
{
	PCONFIG			pCfg	= (PCONFIG) (CONFIG_ADDRESS);
	UINT16_PTR_T	pCheck	= (UINT16_PTR_T) (CONFIG_ADDRESS);
	UINT32_T		u;

	for (u = 0; u < 8; u++) {
		if (*pCheck != 0xBF00) 
			break;

		pCheck++;
	}

	if (u == 8) {
		DBG_PRINTF("%s: Use default config.\r\n", __FUNCTION__);
		
		sConfig.AutoRun				= CFG__ENABLE_AUTO_RUN;
		sConfig.Control				= CFG__CONTROL;
		sConfig.AppSize				= 0;
		sConfig.SRAMUsage			= 0;

		NMemSet((UINT8_PTR_T) &(sConfig.AppName[0]), 0, CONFIG_STR_LEN);
		NStrCopy((char *) &(sConfig.AppName[0]), DEF_APP_NAME, CONFIG_STR_LEN);

		return FALSE;
	}

	NMemCopy((UINT8_PTR_T) &sConfig, (UINT8_PTR_T) pCfg, sizeof(CONFIG));

	return TRUE;
}

BOOL CfgSave()
{
	UINT16_PTR_T	pDst = 0, pSrc = 0;
	UINT32_T		unWrite = 0;
	UINT32_T		u;
	BOOL			fRes = FALSE;
	BOOL			fWriteErr = FALSE;

	FLASH_UNLOCK_MEMORY();
	if (FLASH_IS_MEMORY_LOCKED()) {
		ThdSetLastError(ERR__FAIL_TO_UNLOCK_FLASH);
		return FALSE;
	}

	FLASH_ENABLE_PAGE_ERASE(TRUE);

	FLASH_WRITE_ADDRESS(CONFIG_WRITE_ADDRESS);
	FLASH_START_ERASE();
	while (FLASH_IS_BUSY());
	if (FLASH_IS_END_OF_OPERATION() == FALSE) {
		FLASH_ENABLE_PAGE_ERASE(FALSE);
		FLASH_LOCK_MEMORY();
		
		ThdSetLastError(ERR__FLASH_OPERATION);
		return FALSE;
	}
	FLASH_CLEAR_END_OF_OPERATION();

	FLASH_ENABLE_PAGE_ERASE(FALSE);

	unWrite = sizeof(CONFIG) / sizeof(UINT16_T);

	pDst = (UINT16_PTR_T) CONFIG_WRITE_ADDRESS;
	pSrc = (UINT16_PTR_T) &sConfig;
	
	FLASH_ENABLE_PROGRAMMING(TRUE);

	for (u = 0; u < unWrite; u++)
	{
		*pDst = *pSrc;
		while (FLASH_IS_BUSY());

		if (FLASH_IS_PROGRAMMING_ERROR() == TRUE) {
			FLASH_CLEAR_PROGRAMMING_ERROR();
			fWriteErr = TRUE;
			ThdSetLastError(ERR__FLASH_PROGRAMMING);
			break;
		}

		if (FLASH_IS_WRITE_PROTECTION_ERROR() == TRUE) {
			FLASH_CLEAR_WRITE_PROTECTION_ERROR();
			fWriteErr = TRUE;
			ThdSetLastError(ERR__FLASH_WRITE_PROTECTION);
			break;
		}

		if (FLASH_IS_END_OF_OPERATION() == TRUE) {
			FLASH_CLEAR_END_OF_OPERATION();
		} else {
			fWriteErr = TRUE;
			ThdSetLastError(ERR__FLASH_OPERATION);
			break;
		}

		pDst++;
		pSrc++;
	}

	FLASH_ENABLE_PROGRAMMING(FALSE);
	FLASH_LOCK_MEMORY();

	if (!fWriteErr) fRes = TRUE;

	return fRes;
}