/*
 * File		: NanoStd.cpp
 * Remark	: Implementation of NanoStd namespace
 *
 */

#include "..\NanoOSApi.h"
#include "NanoStd.h"

namespace NanoStd
{

Output		COut(false);
Input		CIn;
Output		CErr(true);

const char	sHexCharsU[] = { "0123456789ABCDEF" };
const char	sHexCharsL[] = { "0123456789abcdef" };

Output::Output(bool fDbgMode)
{
	m_OutMode	= Decimal;
	m_fDbgMode = fDbgMode;
}

Output::~Output()
{
}

void Output::_WriteString(const char *szString)
{
	int	iLen = ::StrLen(szString);
	if (iLen <= 0) return;

	char *p = const_cast<char *>(szString);

	if (m_fDbgMode) {
		for (int i = 0; i < iLen; i++) {
			DebugCharOut((UINT8_T) *p);
			p++;
		}
	} else {
		WriteToUsbSerial((UINT8_PTR_T) szString, iLen);
	}
}

void Output::_WriteString(SArray<char, 64> &szBuffer)
{
	int	iLen = StrLen(&szBuffer[0]);
	if (iLen <= 0) return;

	char *p = &szBuffer[0];

	if (m_fDbgMode) {
		for (int i = 0; i < iLen; i++) {
			DebugCharOut((UINT8_T) *p);
			p++;
		}
	} else {
		WriteToUsbSerial((UINT8_PTR_T) &szBuffer[0], iLen);
	}
}

void Output::_WriteEndOfLine()
{
	char * szEOL = const_cast<char *>("\r\n");
	if (m_fDbgMode) {
		DebugCharOut((UINT8_T) *szEOL++);
		DebugCharOut((UINT8_T) *szEOL);
	} else {
		WriteToUsbSerial((UINT8_PTR_T) szEOL, 2);
	}
}

void Output::_WriteEndString()
{
	char chEOS = '\0';
	if (m_fDbgMode) {
		DebugCharOut((UINT8_T)chEOS);
	} else {
		WriteToUsbSerial((UINT8_PTR_T) &chEOS, 1);
	}
}

void Output::_WriteChar(char Char)
{
	UINT8_T	uCh = (UINT8_T) Char;
	if (m_fDbgMode) {
		DebugCharOut(uCh);
	} else {
		WriteToUsbSerial(&uCh, 1);
	}
}

void Output::_WriteDecimal(unsigned int uiNumber)
{
	unsigned int	uDiv = 0, uMod = 0, uRes = 0;
	char			buf[128];
	char *			pc;
	int				iRes = 0;

	buf[0] = 0;
	pc = &buf[1];
	uRes = uiNumber;

	do {
		uDiv = uRes / 10;
		uMod = uRes - (uDiv * 10);
		*pc++ = sHexCharsU[uMod];
		uRes = uDiv;
	} while (uDiv >= 10);
	if (uDiv != 0)
		*pc = sHexCharsU[uDiv];
	else
		pc--;

	while (*pc != '\0') {
		_WriteChar(*pc);
		pc--;
		iRes++;
	}
}

void Output::_WriteDecimal(int iNumber)
{
	unsigned int u = static_cast<unsigned int>(iNumber);
	if (iNumber < 0) {
		u = ~u;
		u++;
		_WriteChar('-');
	}
	_WriteDecimal(u);
}

void Output::_WriteHexa(unsigned int uiNumber, bool bIsHexCapital, unsigned int uDigit)
{
	char			buf[8];
	char			*pc;
	unsigned int	i, uMax;
	int				ch;

	uMax = (uDigit < sizeof(buf)) ? uDigit : sizeof(buf);

	pc = &buf[0];
	for (i = 0; i < uMax; i++) {
		ch = uiNumber & 0xF;
		if (bIsHexCapital) {
			*pc++ = sHexCharsU[ch];
		} else {
			*pc++ = sHexCharsL[ch];
		}
		uiNumber >>= 4;
	}

	pc--;

	for (i = 0; i < uMax; i++) {
		_WriteChar(*pc);
		pc--;
	}
}

Output& Output::operator <<(char ch)
{
	_WriteChar(ch);
	return *this;
}

Output& Output::operator <<(const char *szText)
{
	_WriteString(szText);
	return *this;
}

Output& Output::operator <<(NanoStd::SArray<char,64> &szBuffer)
{
	_WriteString(szBuffer);
	return *this;
}

Output& Output::operator <<(NanoStd::OutType OutT)
{
	switch (OutT)
	{
	case EndLine:
		_WriteEndOfLine();
		break;

	case EndString:
		_WriteEndString();
		break;

	default:
		break;
	}

	return *this;
}

Output& Output::operator <<(NanoStd::OutMode OutM)
{
	m_OutMode = OutM;
	return *this;
}

Output& Output::operator <<(unsigned int uiNumber)
{
	switch (m_OutMode)
	{
	case Decimal:
		_WriteDecimal(uiNumber);
		break;

	case Hexa:
		_WriteHexa(uiNumber, true, 8);
		break;

	default:
		break;
	}

	return *this;
}

Output& Output::operator <<(int iNumber)
{
	switch (m_OutMode)
	{
	case Decimal:
		_WriteDecimal(iNumber);
		break;

	case Hexa:
		{
			unsigned int u = static_cast<unsigned int>(iNumber);
			_WriteHexa(u, true, 8);
		}
		break;

	default:
		break;
	}

	return *this;
}

Input::Input()
{
}

Input::~Input()
{
}

void Input::_ReadChar(char & ch)
{
	UINT8_T	uch = 0;
	BOOL	fRes = ReadFromUsbSerial(&uch, 1, 0);
	if (fRes) {
		ch = static_cast<char>(uch);
	}
}

/* Warning! This function doesn't provide buffer length checking */
void Input::_ReadString(char szStrBuf[])
{
	char *pBuf = &szStrBuf[0];
	char chNL = '\n';
	BOOL	fRes = FALSE;
	do {
		fRes = ReadFromUsbSerial(reinterpret_cast<UINT8_PTR_T>(pBuf), 1, 0);
		if (fRes) {
			WriteToUsbSerial(reinterpret_cast<UINT8_PTR_T>(pBuf), 1);
			if (*pBuf == '\r') {
				WriteToUsbSerial(reinterpret_cast<UINT8_PTR_T>(&chNL), 1);
				pBuf++;
				*pBuf = '\0';
				break;
			}
			pBuf++;
		}
	} while (fRes);
}

void Input::_ReadString(SArray<char, 64> &Buffer)
{
	char *			pBuf = &Buffer[0];
	char			chNL = '\n';
	BOOL			fRes = FALSE;
	unsigned int	uiCount = 0;

	Buffer.Clear();
	do {
		fRes = ReadFromUsbSerial(reinterpret_cast<UINT8_PTR_T>(pBuf), 1, 0);
		if (fRes) {
			WriteToUsbSerial(reinterpret_cast<UINT8_PTR_T>(pBuf), 1);
			if (*pBuf == '\r') {
				WriteToUsbSerial(reinterpret_cast<UINT8_PTR_T>(&chNL), 1);
				pBuf++;
				*pBuf = '\0';
				break;
			}
			if (++uiCount >= Buffer.Size())
				break;

			pBuf++;
		}
	} while (fRes);
}

void Input::_ReadNumber(unsigned int &uiNumber)
{
	MemSet((UINT8_PTR_T) &m_szBuf[0], 0, sizeof(m_szBuf));

	BOOL	fRes = FALSE;

	char *p = &m_szBuf[0];
	unsigned int uCount = 0;
	char	chNL = '\n';
	while (true) {
		fRes = ReadFromUsbSerial((UINT8_PTR_T) p, 1, 0);
		if (!fRes) break;

		WriteToUsbSerial((UINT8_PTR_T) p, 1);
		if (*p == '\r') {
			*p = '\0';
			p--;
			WriteToUsbSerial((UINT8_PTR_T) &chNL, 1);
			break;
		}

		if (++uCount >= (sizeof(m_szBuf) - 1)) break;
		p++;
	}

	if (!fRes) return;

	uiNumber = 0;
	_StringToUInt(p, uCount, uiNumber);
}

void Input::_ReadNumber(int & iNumber)
{
	MemSet((UINT8_PTR_T) &m_szBuf[0], 0, sizeof(m_szBuf));

	BOOL	fRes = FALSE;

	char *p = &m_szBuf[0];
	unsigned int uiCount = 0;
	char	chNL = '\n';
	while (true) {
		fRes = ReadFromUsbSerial(reinterpret_cast<UINT8_PTR_T>(p), 1, 0);
		if (!fRes) break;

		WriteToUsbSerial(reinterpret_cast<UINT8_PTR_T>(p), 1);
		if (*p == '\r') {
			*p = '\0';
			p--;
			WriteToUsbSerial(reinterpret_cast<UINT8_PTR_T>(&chNL), 1);
			break;
		}

		if (++uiCount >= (sizeof(m_szBuf) - 1)) break;
		p++;
	}

	if (!fRes) return;

	if (m_szBuf[0] == '-') uiCount--;

	unsigned int	uiRes = 0;
	if (_StringToUInt(p, uiCount, uiRes)) {
		if (m_szBuf[0] == '-') 
		{
			uiRes = ~uiRes;
			uiRes++;
		}
		else if (uiRes > 0x7FFFFFFF) 
		{
			uiRes = 0x7FFFFFFF;
		} 
		else 
		{
			// Do nothing
		}
		iNumber = static_cast<int>(uiRes);
	}
}

bool Input::_StringToUInt(char *pLastChar, unsigned int uiCharCount, unsigned int &uiResult)
{
	if (!pLastChar) return false;

	bool			b = true;
	unsigned int	uNumb = 0, uMul = 0, uiOpRes = 0, uiTemp = 0;
	char *			p = pLastChar;

	for (unsigned int ui = 0; ui < uiCharCount; ui++)
	{
		if ((*p >= '0') && (*p <= '9')) {
			uNumb = (static_cast<unsigned int>(*p) - static_cast<unsigned int>('0'));
			uMul = 1;
			for (unsigned int u = 0; u < ui; u++) {
				
				/*
				 * Simulate: uMul = uMul * 10;
				 */
				__asm volatile
					(
					"MOV.W R4, 10;"
					"MULS R4, %1, R4;"
					"MOV.W %0, R4;"
					: "=r" (uMul) : "r" (uMul)
					);
			}

			/*
			 * Simulate: uiTemp += (uNumb * uMul);
			 */
			__asm
				(
				"MOV.W R6, 0;"
				"ADDS R6, 10;"		// Clear flags
				"MOV.W R6, %4;"
				"MOV.W R5, %3;"
				"MOV.W R4, %2;"
				"MUL R5, R6;"
				"IT CC;"
				"ADDSCC.W R4, R5;"
				"MRS %0, APSR;"
				"IT CC;"
				"MOVCC.W %1, R4;"
				: "=r" (uiOpRes), "=r" (uiTemp) : "r" (uiTemp), "r" (uMul), "r" (uNumb)
				);

			if (uiOpRes & (CARRY_FLAG)) {
				b = false;
				break;
			}
			p--;
		} else {
			b = false;
			break;
		}
	}

	uiResult = uiTemp;

	return b;
}

Input& Input::operator >>(char szStrBuf[])
{
	_ReadString(szStrBuf);
	return *this;
}

Input& Input::operator >>(SArray<char, 64> &Buffer)
{
	_ReadString(Buffer);
	return *this;
}

Input& Input::operator >>(unsigned int &uiNumber)
{
	unsigned int	uiRes = 0;

	_ReadNumber(uiRes);
	uiNumber = uiRes;

	return *this;
}

Input& Input::operator >>(char &ch)
{
	_ReadChar(ch);
	return *this;
}

Input& Input::operator >>(int &iNumber)
{
	_ReadNumber(iNumber);
	return *this;
}

};
