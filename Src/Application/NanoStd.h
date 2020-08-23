/*
 * File    : NanoStd.h
 * Remark  : NanoStd header (for namespace and classes). 
 *			 NanoStd provide basic input output via USB Serial IO port. This class is similar
 *			 to basic input output in <iostream> header but simpler and specifically for NanoOS.
 *           In addition, this header provide a "common class template", such as String and List.
 *           Most of these templates are designed be used in system that does not provide dynamic 
 *           memory allocation such as NanoOS.
 *
 */

#pragma once
#include "NanoOSApi.h"

namespace NanoStd
{
template <typename Type, unsigned int MaxItem>
class List;

template <typename Type, unsigned int MaxItem>
class Queue;

template <typename Type>
class _Container
{
public:
	_Container<Type> *	m_FLink;
	_Container<Type> *	m_BLink;
	Type				m_Object;
	bool				m_IsFilled;

	_Container<Type>()
	{
		m_FLink = this;
		m_BLink = this;
		m_IsFilled = false;
	}

	~_Container<Type>(){}
};

template <typename Type, unsigned int MaxItem>
class _Iterator
{
	_Container<Type> *	m_ContPtr;
	Type				m_Dummy;
	bool				m_IsError;

public:
	_Iterator<Type, MaxItem>()
	{
		m_ContPtr = 0;
		m_IsError = false;
	}

	~_Iterator<Type, MaxItem>(){};

	_Iterator<Type, MaxItem> operator =(_Iterator<Type, MaxItem> RightIterator)
	{
		m_ContPtr = RightIterator.m_ContPtr;
		return *this;
	}

	_Iterator<Type, MaxItem> operator ++(int i)		// As postfix
	{
		m_IsError = false;
		if (!m_ContPtr) {
			m_IsError = true;
		} else {
			m_ContPtr = m_ContPtr->m_FLink;
		}
		return *this;
	}

	_Iterator<Type, MaxItem> operator --(int i)		// As postfix
	{
		m_IsError = false;
		if (!m_ContPtr) {
			m_IsError = true;
		} else {
			m_ContPtr = m_ContPtr->m_BLink;
		}
		return *this;
	}

	Type operator *()
	{
		m_IsError = false;
		if (!m_ContPtr) {
			m_IsError = true;
			return m_Dummy;
		}
		return m_ContPtr->m_Object;
	}

	Type * operator ->()
	{
		m_IsError = false;
		if (!m_ContPtr) {
			m_IsError = true;
			return &m_Dummy;
		}
		return &(m_ContPtr->m_Object);
	}

	bool operator ==(_Iterator<Type, MaxItem> RightIterator)
	{
		if (m_ContPtr == RightIterator.m_ContPtr)
			return true;

		return false;
	}

	bool operator !=(_Iterator<Type, MaxItem> RightIterator)
	{
		if (m_ContPtr != RightIterator.m_ContPtr)
			return true;

		return false;
	}

	bool IsError()
	{
		return m_IsError;
	}

	friend class List<Type, MaxItem>;
	friend class Queue<Type, MaxItem>;
};

template <typename Type, unsigned int MaxItem>
class Queue
{
	_Container<Type>			m_Containers[MaxItem];
	_Container<Type> *			m_GetPtr;
	_Container<Type> *			m_PutPtr;
	_Container<Type> *			m_TmpFLink;
	_Container<Type> *			m_TmpBLink;
	_Iterator<Type, MaxItem>	m_Iterator;
	Type						m_Dummy;
	bool						m_IsError;
	unsigned int				m_Count;

	void _Reset()
	{
		// Build circular buffer
		for (unsigned int u = 0; u < MaxItem; u++)
		{
			if (u == (MaxItem - 1)) {
				m_Containers[u].m_FLink = &m_Containers[0];
				m_PutPtr = &m_Containers[u];
			} else {
				m_Containers[u].m_FLink = &m_Containers[(u + 1)];
			}
			if (u == 0) {
				m_Containers[u].m_BLink = &m_Containers[(MaxItem - 1)];
			} else {
				m_Containers[u].m_BLink = &m_Containers[(u - 1)];
			}
			m_Containers[u].m_IsFilled = false;
		}
		m_GetPtr = 0;
		m_TmpFLink = 0;
		m_TmpBLink = 0;
		m_IsError = false;
		m_Count = 0;
	}

public:
	typedef _Iterator<Type, MaxItem> Iterator;

	Queue<Type, MaxItem>()
	{
		_Reset();
	}

	~Queue<Type, MaxItem>() {}

	Iterator Begin()
	{
		m_Iterator.m_ContPtr = m_GetPtr;
		return m_Iterator;
	}

	Iterator End()
	{
		m_Iterator.m_ContPtr = 0;
		return m_Iterator;
	}

	bool Put(const Type &Item)
	{
		if (m_TmpFLink) {
			if (m_TmpFLink == m_GetPtr) {
				return false;
			}

			m_PutPtr->m_FLink = m_TmpFLink;		// Link forward linkage
			m_TmpFLink->m_BLink = m_TmpBLink;	// Link backward linkage
		}
		m_PutPtr = m_PutPtr->m_FLink;

		m_PutPtr->m_Object = Item;

		if (m_GetPtr == 0) m_GetPtr = m_PutPtr;
		
		m_TmpFLink = m_PutPtr->m_FLink;
		m_TmpBLink = m_TmpFLink->m_BLink;
		m_TmpFLink->m_BLink = 0;			// Break backward linkage
		m_PutPtr->m_FLink = 0;				// Break forward linkage
		
		m_Count++;
		return true;
	}

	bool Get(Type &Item)
	{
		if (m_GetPtr == 0)
			return false;

		Item = m_GetPtr->m_Object;
		
		m_GetPtr = m_GetPtr->m_FLink;	// 'Move forward'
		
		m_Count--;
		return true;
	}

	unsigned int Count()
	{
		return m_Count;
	}

	void Reset()
	{
		_Reset();
	}
};

template <typename Type, unsigned int MaxItem>
class List
{
	_Container<Type>			m_Containers[MaxItem];
	_Container<Type> *			m_FrontPtr;			// Front container pointer
	_Container<Type> *			m_BackPtr;			// Back container pointer
	_Container<Type> *			m_TmpPtr;			// Temporary container pointer
	_Iterator<Type, MaxItem>	m_Iterator;
	Type						m_Dummy;
	bool						m_IsError;
	unsigned int				m_Count;

	void _FindEmptyContainer()
	{
		m_TmpPtr = 0;
		for (unsigned int u = 0; u < MaxItem; u++)
		{
			if (m_Containers[u].m_IsFilled == false)
			{
				m_TmpPtr = &m_Containers[u];
				break;
			}
		}
	}

public:
	typedef _Iterator<Type, MaxItem>	Iterator;

	List<Type, MaxItem>()
	{
		m_TmpPtr = 0;
		m_FrontPtr = 0;
		m_BackPtr = 0;
		m_IsError = false;
		m_Count = 0;
	}

	~List<Type, MaxItem>() {}

	List<Type, MaxItem> operator [](unsigned int uiIndex)
	{
		m_IsError = false;
		if (uiIndex >= m_Count) {
			m_IsError = true;
			return m_Dummy;
		}

		return m_Containers[uiIndex];
	}

	Iterator Begin()
	{
		m_Iterator.m_ContPtr = m_FrontPtr;
		return m_Iterator;
	}

	Iterator End()
	{
		m_Iterator.m_ContPtr = 0;
		return m_Iterator;
	}

	Iterator RevBegin()
	{
		m_Iterator.m_ContPtr = m_BackPtr;
		return m_Iterator;
	}

	Iterator RevEnd()
	{
		m_Iterator.m_ContPtr = 0;
		return m_Iterator;
	}

	bool PushBack(const Type &Item)
	{
		_FindEmptyContainer();
		if (!m_TmpPtr) return false;

		m_TmpPtr->m_Object = Item;
		m_TmpPtr->m_IsFilled = true;

		if (m_BackPtr == 0)
		{
			m_FrontPtr = m_TmpPtr;
			m_BackPtr = m_TmpPtr;
			m_TmpPtr->m_BLink = 0;
			m_TmpPtr->m_FLink = 0;
		}
		else
		{			
			m_TmpPtr->m_FLink = 0;
			m_TmpPtr->m_BLink = m_BackPtr;
			m_BackPtr->m_FLink = m_TmpPtr;
			m_BackPtr = m_TmpPtr;
		}
		
		m_Count++;
		return true;
	}

	bool PopBack(Type &RetItem)
	{
		if (m_BackPtr == 0)
			return false;

		RetItem = m_BackPtr->m_Object;
		m_BackPtr->m_IsFilled = false;

		if (m_BackPtr == m_FrontPtr)
		{
			m_BackPtr = 0;
			m_FrontPtr = 0;
		}
		else
		{
			m_BackPtr = m_BackPtr->m_BLink;
			m_BackPtr->m_FLink = 0;
		}

		m_Count--;
		return true;
	}

	bool PopFront(Type &RetItem)
	{
		if (m_FrontPtr == 0)
			return false;

		RetItem = m_FrontPtr->m_Object;
		m_FrontPtr->m_IsFilled = false;

		if (m_FrontPtr == m_BackPtr) 
		{
			m_FrontPtr = 0;
			m_BackPtr = 0;
		}
		else
		{
			m_FrontPtr = m_FrontPtr->m_FLink;
			m_FrontPtr->m_BLink = 0;
		}

		m_Count--;
		return true;
	}

	bool PushFront(const Type &Item)
	{
		_FindEmptyContainer();
		if (!m_TmpPtr) return false;

		m_TmpPtr->m_Object = Item;
		m_TmpPtr->m_IsFilled = true;

		if (m_FrontPtr == 0)
		{
			m_FrontPtr = m_TmpPtr;
			m_BackPtr = m_TmpPtr;
			m_TmpPtr->m_FLink = 0;
			m_TmpPtr->m_BLink = 0;
		}
		else
		{
			m_TmpPtr->m_FLink = m_FrontPtr;
			m_TmpPtr->m_BLink = 0;
			m_FrontPtr->m_BLink = m_TmpPtr;
			m_FrontPtr = m_TmpPtr;
		}
		m_Count++;
		return true;
	}

	bool Remove(const Type &Item)
	{
		bool	b = false;

		if (!m_FrontPtr)
			return false;

		m_TmpPtr = m_FrontPtr;
		do {
			if (m_TmpPtr->m_Object == Item)
			{
				if (m_FrontPtr == m_BackPtr) {
					m_FrontPtr = 0;
					m_BackPtr = 0;
				} else {
					if (m_TmpPtr == m_FrontPtr)
					{
						m_FrontPtr = m_FrontPtr->m_FLink;
						m_FrontPtr->m_BLink = 0;
					}
					else if (m_TmpPtr == m_BackPtr)
					{
						m_BackPtr = m_BackPtr->m_BLink;
						m_BackPtr->m_FLink = 0;
					}
					else
					{
						m_TmpPtr->m_BLink->m_FLink = m_TmpPtr->m_FLink;
						m_TmpPtr->m_FLink->m_BLink = m_TmpPtr->m_BLink;
					}
				}
				m_TmpPtr->m_IsFilled = false;
				m_Count--;
				b = true;
				break;
			}
			m_TmpPtr = m_TmpPtr->m_FLink;
		} while (m_TmpPtr != 0);

		return b;
	}

	bool First(Type &Item)
	{
		if (m_FrontPtr == 0) 
			return false;

		Item = m_FrontPtr->m_Object;
		return true;
	}

	bool Last(Type &Item)
	{
		if (!m_BackPtr) 
			return false;

		Item = m_BackPtr->m_Object;
		return true;
	}

	bool Find(const Type &Item)
	{
		bool b = false;
		m_TmpPtr = m_FrontPtr;

		while (m_TmpPtr != 0)
		{
			if (m_TmpPtr->m_IsFilled) {
				if (m_TmpPtr->m_Object == Item) {
					b = true;
					break;
				}
			}

			m_TmpPtr = m_TmpPtr->m_FLink;
		}

		return b;
	}

	bool IsError()
	{
		return m_IsError();
	}

	unsigned int Count()
	{
		return m_Count;
	}

	void Clear()
	{
		if (m_FrontPtr)
		{
			do {
				m_FrontPtr->m_IsFilled = false;
				m_FrontPtr = m_FrontPtr->m_FLink;
			} while (m_FrontPtr != 0);

			m_BackPtr = 0;
			m_Count = 0;
		}
	}
};

template <typename Type, unsigned int Length>
void MemSet(Type *pVar, const unsigned char Value)
{
	// Assume the Type size is aligned in byte
	if (!pVar) return;
	unsigned char *pChar = reinterpret_cast<unsigned char *>(pVar);
	unsigned int MaxLen = sizeof(Type) * Length;
	for (unsigned int u = 0; u < MaxLen; u++)
	{
		*pChar++ = Value;
	}
}

template <typename Type>
void MemSet(Type *pVar, unsigned int Length, const unsigned char Value)
{
	// Assume the Type size is aligned in byte
	if (!pVar) return;
	unsigned char *pChar = reinterpret_cast<unsigned char *>(pVar);
	int MaxLen = sizeof(Type) * Length;
	for (int i = 0; i < MaxLen; i++)
	{
		*pChar++ = Value;
	}
}

template <typename Type, unsigned int Length>
void MemCopy(Type *pDst, Type *pSrc)
{
	if ((!pDst) || (!pSrc)) return;
	for (unsigned int u = 0; u < Length; u++)
	{
		*pDst++ = *pSrc++;
	}
}

template <typename Type>
void MemCopy(Type *pDst, Type *pSrc, unsigned int Length)
{
	if ((!pDst) || (!pSrc)) return;
	for (unsigned int u = 0; u < Length; u++)
	{
		*pDst++ = *pSrc++;
	}
}

template <typename Type, unsigned int MaxLength>
class String
{
	Type				m_Buff[MaxLength];
	Type				m_Dummy;
	unsigned int		m_Length;
	bool				m_IsError;
	unsigned int		m_MaxIndex;

public:
	String<Type, MaxLength>()
	{
		MemSet<Type, MaxLength>(&m_Buff[0], '\0');
		m_Length = 0;
		m_MaxIndex = MaxLength - 2;
	}

	~String<Type, MaxLength>() {}

	unsigned int Length()
	{
		return m_Length;
	}

	unsigned int MaxBuffer()
	{
		return (MaxLength - 1);
	}

	String<Type, MaxLength> operator =(const Type *szString)
	{
		MemSet<Type, MaxLength>(&m_Buff[0], '\0');
		m_Length = 0;

		Type *pSrc = const_cast<Type *>(szString);
		Type *pDst = &m_Buff[0];
		while (*pSrc != 0)
		{
			*pDst++ = *pSrc++;
			m_Length++;

			if (m_Length >= (MaxLength - 1)) break;
		}

		return *this;
	}

	String<Type, MaxLength> operator +=(const Type* szString)
	{
		Type *pSrc = const_cast<Type *>(szString);
		Type *pDst = &m_Buff[0];
		pDst += m_Length;
		while (*pSrc != 0)
		{
			if (m_Length >= (MaxLength - 1)) break;

			*pDst++ = *pSrc++;
			m_Length++;
		}

		return *this;
	}

	String<Type, MaxLength> operator =(String<Type, MaxLength> RightString)
	{
		MemCopy<Type, MaxLength>(&m_Buff[0], &RightString.m_Buff[0]);
		m_Length = RightString.m_Length;
		return *this;
	}

	String<Type, MaxLength> operator +=(String<Type, MaxLength> RightString)
	{
		Type *pSrc = &RightString.m_Buff[0];
		Type *pDst = &m_Buff[0];
		pDst += m_Length;
		while (*pSrc != 0)
		{
			if (m_Length >= (MaxLength - 1)) break;

			*pDst++ = *pSrc++;
			m_Length++;
		}

		return *this;
	}

	String<Type, MaxLength> operator +=(Type Value)
	{
		if (m_Length < (MaxLength - 1)) {
			m_Buff[m_Length] = Value;
			m_Length++;
		}

		return *this;
	}

	Type& operator [](unsigned int Index)
	{
		m_IsError = false;
		if (Index > (MaxLength - 1)) {
			m_IsError = true;
			return m_Dummy;
		}

		return m_Buff[Index];
	}

	void Clear()
	{
		MemSet<Type, MaxLength>(&m_Buff[0], '\0');
		m_Length = 0;
	}

	const Type * CStr() const
	{
		return &m_Buff[0];
	}

	Type * GetBuffer()		// Watch out for buffer length when use this
	{
		return &m_Buff[0];
	}

	bool AppendChar(Type ch)
	{
		if (m_Length >= (MaxLength - 1))
			return false;
		m_Buff[m_Length] = ch;
		m_Length++;
		return true;
	}

	char * GetLastCharPtr()
	{
		return &m_Buff[m_Length - 1];
	}

	unsigned int AvailableSpace()
	{
		return ((MaxLength - 1) - m_Length);
	}

	bool Insert(Type *pItemPos, const Type & Item)
	{
		// Assume memory 'grows' from low to high address
		if ((pItemPos < &m_Buff[0]) ||
			(pItemPos >= &m_Buff[MaxLength - 1]))
		{
			return false;
		}

		Type * pItem = &m_Buff[m_Length - 1];
		if (pItemPos > pItem) {
			pItem++;
			*pItem = Item;
			m_Length++;
			return true;
		}

		pItem = pItemPos;

		int iMax = static_cast<int>(&m_Buff[m_Length - 1] - pItem) + 1;
		Type CurItem = *pItem;
		Type NextItem;

		for (int i = 0; i < iMax; i++)
		{
			pItem++;
			NextItem = *pItem;
			*pItem = CurItem;
			CurItem = NextItem;
		}

		*pItemPos = Item;
		m_Length++;

		return true;
	}

	bool Insert(unsigned int Index, const Type & Item)
	{
		if (m_MaxIndex < Index) return false;

		if (Index > (m_Length - 1)) {
			m_Buff[m_Length] = Item;
			m_Length++;
			return true;
		}

		Type * pItem = &m_Buff[Index];
		int iMax = (m_Length - Index) + 1;
		Type CurItem = *pItem;
		Type NextItem;

		for (int i = 0; i < iMax; i++)
		{
			pItem++;
			NextItem = *pItem;
			*pItem = CurItem;
			CurItem = NextItem;
		}

		m_Buff[Index] = Item;
		m_Length++;

		return true;
	}

	bool Remove(Type *pItemPos, Type & RetItem)
	{
		// Assume memory 'grows' from low to high address
		if ((pItemPos < &m_Buff[0]) ||
			(pItemPos >= &m_Buff[MaxLength - 1]))
		{
			return false;
		}

		if (*pItemPos == '\0') return false;
		if (m_Length == 0) return false;

		Type * pItem = &m_Buff[m_Length - 1];
		if (pItemPos >= pItem) {
			RetItem = *pItem;
			MemSet<Type>(pItem, 1, '\0');
			m_Length--;
			return true;
		}

		RetItem = *pItemPos;

		pItem = pItemPos;
		int iMax = static_cast<int>(&m_Buff[m_Length - 1] - pItem);
		Type *pCurItem;

		for (int i = 0; i < iMax; i++)
		{
			pCurItem = pItem++;
			*pCurItem = *pItem;
		}

		MemSet<Type>(pItem, 1, '\0');
		m_Length--;

		return true;
	}

	bool Remove(unsigned int Index, Type & RetItem)
	{
		if (m_MaxIndex < Index) return false;
		if (m_Buff[Index] == '\0') return false;

		Type * pItem;

		if (Index >= (m_Length - 1)) {
			pItem = &m_Buff[m_Length - 1];
			RetItem = *pItem;
			MemSet<Type>(pItem, 1, '\0');
			m_Length--;
			return true;
		}

		pItem = &m_Buff[Index];
		int iMax = (m_Length - Index);
		Type * pCurItem;

		RetItem = *pItem;
		m_Length--;

		for (int i = 0; i < iMax; i++)
		{
			pCurItem = pItem++;
			*pCurItem = *pItem;
		}

		MemSet<Type>(pItem, 1, '\0');

		return true;
	}
};
	
template <typename Type, unsigned int NumbOfElements>
class Array
{
	Type				m_Elms[NumbOfElements];
	Type				m_Dummy;
	bool				m_IsError;

public:
	Array<Type, NumbOfElements>()
	{
		MemSet<Type, NumbOfElements>(&m_Elms[0], '\0');
		m_IsError = false;
	}

	~Array<Type, NumbOfElements>(){}

	Type& operator [](unsigned int Index)
	{
		m_IsError = false;
		if (Index >= NumbOfElements) {
			m_IsError = true;
			return m_Dummy;
		}

		return m_Elms[Index];
	}

	Array<Type, NumbOfElements> operator =(Array<Type, NumbOfElements> RightArray)
	{
		for (unsigned int u = 0; u < NumbOfElements; u++)
		{
			m_Elms[u] = RightArray.m_Elms[u];
		}
		return *this;
	}

	bool operator ==(Array<Type, NumbOfElements> RightArray)
	{
		for (unsigned int u = 0; u < NumbOfElements; u++)
		{
			if (m_Elms[u] != RightArray.m_Elms[u])
				return false;
		}

		return true;
	}

	bool operator !=(Array<Type, NumbOfElements> RightArray)
	{
		for (unsigned int u = 0; u < NumbOfElements; u++)
		{
			if (m_Elms[u] != RightArray.m_Elms[u])
				return true;
		}

		return false;
	}

	Type * operator &()
	{
		return &m_Elms[0];
	}

	unsigned int Size()
	{
		return NumbOfElements;
	}

	void Clear() 
	{
		MemSet<Type, NumbOfElements>(&m_Elms[0], 0); 
	}
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
	void _WriteString(Array<char, 64> &szBuffer);
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
	Output& operator<<(Array<char, 64> &szBuffer);
	Output& operator<<(OutType OutT);
	Output& operator<<(OutMode OutM);
	Output& operator<<(unsigned int uiNumber);
	Output& operator<<(int iNumber);
	Output& operator<<(long long llNumber);
	Output& operator<<(unsigned long long llNumber);
};

class Input
{
private:
	char	m_szBuf[64];
	void _ReadChar(char & ch);

	/* Warning! This function doesn't provide buffer length checking */
	void _ReadString(char szStrBuf[]);

	void _ReadString(Array<char, 64> &Buffer);

	void _ReadNumber(unsigned int & uiNumber);
	void _ReadNumber(int & iNumber);
	bool _StringToUInt(char *pLastChar, unsigned int uiCharCount, unsigned int & uiResult);

public:
	Input();
	~Input();

	Input& operator>>(char &ch);

	/* Warning! This operator doesn't provide buffer length checking */
	Input& operator>>(char szStrBuf[]);

	Input& operator>>(Array<char, 64> &Buffer);

	Input& operator>>(unsigned int & uiNumber);
	Input& operator>>(int & iNumber);
};

extern Output		COut;
extern Input		CIn;
extern Output		CErr;
};
