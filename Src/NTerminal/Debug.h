/*
 * File    : Debug.h
 * Remark  : Debug print format definition. 
 *
 */

#include "yvals.h"
#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"

#ifndef DKDEBUG_H
#define DKDEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef UNICODE
# define FUNCT_NAME_STR			__FUNCTIONW__
#else 
# define FUNCT_NAME_STR			__FUNCTION__
#endif

void DbgPrintf(const TCHAR *szFormat, ...);

#ifdef _DEBUG
# define DBG_PRINTF(szFormat, ...)			DbgPrintf(szFormat, __VA_ARGS__)
#else
# define DBG_PRINTF(szFormat, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif // End of __DK_DEBUG_H__
