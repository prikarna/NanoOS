/*
 * File    : Memory.c
 * Remark  : Memory helper routines.
 *
 */

#include "Type.h"

void NMemCopy(UINT8_PTR_T pDst, UINT8_PTR_T pSrc, UINT32_T uLen)
{
	UINT32_T	u;
	
	if ((!pDst) || (!pSrc) || (uLen <= 0)) return;
	
	for (u = 0; u < uLen; u++) {
		*pDst++ = *pSrc++;
	}
}

void NMemSet(UINT8_PTR_T pMem, UINT8_T uSet, UINT32_T uLen)
{
	UINT32_T	u;
	
	if ((!pMem) || (uLen <= 0)) return;
	
	for (u = 0; u < uLen; u++) {
		*pMem++ = uSet;
	}
}

void NMemCopyToPaddedBuffer(UINT8_PTR_T pDestination, UINT8_PTR_T pSource, UINT32_T uSourceLength)
{
	UINT32_T		u, max;
	UINT32_PTR_T	pDst = (UINT32_PTR_T) pDestination;
	UINT16_PTR_T	pSrc = (UINT16_PTR_T) pSource;

	if ((!pDestination) || (!pSource) || (uSourceLength == 0)) return;

	max = uSourceLength / 2;
	for (u = 0; u < max; u++) {
		*pDst = (UINT32_T) *pSrc;
		pDst++;
		pSrc++;
	}

	u = uSourceLength - (max * 2);
	if (u > 0) {
		*pDst = (UINT32_T) *pSrc;
	}
}

void NMemCopyFromPaddedBuffer(UINT8_PTR_T pDestination, UINT8_PTR_T pSource, UINT32_T uDestinationByteLen)
{
	UINT32_T	u, max;
	UINT32_PTR_T	pSrc = (UINT32_PTR_T) pSource;
	UINT16_PTR_T	pDst = (UINT16_PTR_T) pDestination;

	if ((!pDestination) || (!pSource) || (uDestinationByteLen == 0)) return;

	max = uDestinationByteLen / 2;
	for (u = 0; u < max; u++) {
		*pDst++ = (UINT16_T) *pSrc++;
	}

	u = uDestinationByteLen - (max * 2);
	if (u > 0) {
		*pDst = (UINT16_T) *pSrc;
	}
}

int NStrLen(const char * pStr)
{
	int	iRes = 0;
	char *	p = (char *) pStr;

	if (p == 0) return -1;

	while (*p != '\0')
	{
		p++;
		iRes++;
	}

	return iRes;
}

int NStrCmp(const char * pStr1, const char * pStr2)
{
	int		iLen1 = NStrLen(pStr1);
	int		iLen2 = NStrLen(pStr2);
	int		iLen = 0;
	int		i = 0;
	char *	p1 = (char *) pStr1;
	char *	p2 = (char *) pStr2;

	if ((iLen1 <= 0) || (iLen2 <= 0)) return -1;

	if (iLen1 <= iLen2)
		iLen = iLen1;
	else
		iLen = iLen2;

	for (i = 0; i < iLen; i++)
	{
		if (*p1 != *p2) break;
		p1++;
		p2++;
	}

	if ((iLen1 == iLen2) && (i == iLen))
		return 0;

	return iLen;
}

int NStrCopy(char * pBuffer, const char * pStr, int iBufferLen)
{
	int		iLen = NStrLen(pStr);
	int		i;
	char *	pDst;
	char *	pSrc;

	if (iLen <= 0) return 0;
	if (iBufferLen <= 0) return 0;
	if (pBuffer == 0) return 0;

	iLen = ((iLen > iBufferLen) ? iLen : iBufferLen);
	if (iLen <= 1) return 0;

	iLen--;
	pDst = pBuffer;
	pSrc = (char *) pStr;
	for (i = 0; i < iLen; i++) {
		*pDst++ = *pSrc++;
	}

	return iLen;
}