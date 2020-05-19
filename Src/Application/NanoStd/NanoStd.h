/*
 * File    : NanoStd.h
 * Remark  : NanoStd header (for namespace and classes). 
 *			 NanoStd provide basic input output via USB Serial IO port. This class is similar
 *			 to basic input output in <iostream> header but simpler and specifically for NanoOS.
 *
 */

#pragma once
#include "..\NanoOSApi.h"

#define OVERFLOW_FLAG		0x10000000
#define CARRY_FLAG			0x20000000
#define ZERO_FLAG			0x40000000
#define NEGATIVE_FLAG		0x80000000
#define NZ_FLAGS			(NEGATIVE_FLAG | ZERO_FLAG)
#define STATUS_FLAGS		(OVERFLOW_FLAG | CARRY_FLAG | ZERO_FLAG | NEGATIVE_FLAG)

namespace NanoStd
{
	/* Safer array class template */
	template <typename Type, unsigned int NumberOfElements>
	class SArray
	{
	private:
		Type			m_Elements[NumberOfElements + 1];	// +1 to compensate error
		bool			m_bError;

	public:
		SArray<Type, NumberOfElements>() 
		{
			MemSet((UINT8_PTR_T) &m_Elements[0], 0, sizeof(m_Elements));
		};

		~SArray<Type, NumberOfElements>() {};

		Type& operator[](unsigned int iIndex)
		{
			m_bError = false;
			if (iIndex >= NumberOfElements) {
				m_bError = true;
				return m_Elements[NumberOfElements];
			}
			return m_Elements[iIndex];
		};

		unsigned int Size() { return NumberOfElements; }
		bool Error() { return m_bError; }
		void Clear() { MemSet((UINT8_PTR_T) &m_Elements[0], 0, sizeof(m_Elements)); }
	};

	enum OutType {
		EndLine,
		EndString
	};

	enum OutMode {
		Decimal,
		Hexa
	};

	class Output
	{
	private:
		OutMode		m_OutMode;
		bool		m_fDbgMode;

		void _WriteString(const char *szString);
		void _WriteString(SArray<char, 64> &szBuffer);
		void _WriteDecimal(unsigned int uiNumber);
		void _WriteDecimal(int iNumber);
		void _WriteHexa(unsigned int uiNumber, bool bIsHexCapital, unsigned int uDigit);
		void _WriteEndOfLine();
		void _WriteEndString();
		void _WriteChar(char Char);

	public:
		Output(bool fDbgMode);
		~Output();

		Output& operator<<(char ch);
		Output& operator<<(const char *szText);
		Output& operator<<(SArray<char, 64> &szBuffer);
		Output& operator<<(OutType OutT);
		Output& operator<<(OutMode OutM);
		Output& operator<<(unsigned int uiNumber);
		Output& operator<<(int iNumber);
	};

	class Input
	{
	private:
		char	m_szBuf[64];
		void _ReadChar(char & ch);

		/* Warning! This function doesn't provide buffer length checking */
		void _ReadString(char szStrBuf[]);

		void _ReadString(SArray<char, 64> &Buffer);

		void _ReadNumber(unsigned int & uiNumber);
		void _ReadNumber(int & iNumber);
		bool _StringToUInt(char *pLastChar, unsigned int uiCharCount, unsigned int & uiResult);

	public:
		Input();
		~Input();

		Input& operator>>(char &ch);

		/* Warning! This operator doesn't provide buffer length checking */
		Input& operator>>(char szStrBuf[]);

		Input& operator>>(SArray<char, 64> &Buffer);

		Input& operator>>(unsigned int & uiNumber);
		Input& operator>>(int & iNumber);
	};

	extern Output		COut;
	extern Input		CIn;
	extern Output		CErr;
};
