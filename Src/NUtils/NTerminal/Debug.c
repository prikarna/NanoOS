/*
 * File    : Debug.c
 * Remark  : Debug output in formatted string implementation.
 *
 */

#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"

void DbgPrintf(const TCHAR * szFormat, ...)
{
	va_list			al;
	TCHAR			szBuf[1024] = {0};

	va_start(al, szFormat);
	StringCbVPrintf(szBuf, sizeof(szBuf), szFormat, al);
	va_end(al);

	OutputDebugString(szBuf);
}