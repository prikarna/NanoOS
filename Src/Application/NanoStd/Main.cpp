/*
 * File    : Main.cpp
 * Remark  : Demonstrate how to write NanoOS application in C++ using NanoStd namespace.
 */

#include "..\NanoStd.h"

using namespace NanoStd;

int main(int , char *[])
{
	COut << "Hello world." << EndLine;

	COut << EndLine;

	COut << "++ Test ouput ++" << EndLine;
	int i = 17;
	unsigned int ui = 17;
	COut << Decimal << "Var. i = " << i << " (0x" << Hexa << i << ')' << EndLine;
	COut << Decimal << "Var. ui = " << ui << " (0x" << Hexa << ui << ')' << EndLine;
	i = -17;
	COut << Decimal << "Var. i (after being changed) = " << i << " (0x" << Hexa << i << ')' << EndLine;

	int j = 9;
	int k = i + j;
	COut << Decimal << i << " + " << j << " = " << k << EndLine;

	k = i * j;
	COut << Decimal << i << " x " << j << " = " << k << EndLine;

	COut << EndLine;

	COut << "++ Test input ++" << EndLine;
	for (int i = 0; i < 2; i++) {
		COut << Decimal << '[' << i << "] Type a number: ";
		int aNumb = 0;
		CIn >> aNumb;
		COut << '[' << i << "] You type: " << aNumb << Hexa << " (0x" << aNumb << ')' << EndLine;
	}

	Array<char, 64>	szBuf;
	COut << "Tell me your name: ";
	CIn >> szBuf;
	COut << "Hello " << szBuf << '.' << EndLine;

	COut << Decimal;	// Set to decimal mode
	COut << "Press <Enter> to continue...";
	CIn >> szBuf;

	COut << "Creating queue" << EndLine;
	Queue<int, 6>		IntQue;
	for (int i = 0; i < 10; i++)
	{
		COut << "Adding " << i+10 << " to the queue." << EndLine;
		IntQue.Put(i+10);
	}
	COut << "Queue count = " << IntQue.Count() << EndLine;
	COut << EndLine;
	COut << "Listing queue : " << EndLine;
	Queue<int, 6>::Iterator ii = IntQue.Begin();
	int n = 0;
	while (ii != IntQue.End())
	{
		COut << "int " << n++ << " is " << *ii << EndLine;
		ii++;
	}
	COut << EndLine;
	COut << "Getting the queue : " << EndLine;
	int iRes;
	n = 0;
	while (IntQue.Get(iRes))
	{
		COut << "int no. " << n++ << " is " << iRes << EndLine;
	}
	COut << "Queue count = " << IntQue.Count() << EndLine;

	COut << EndLine;
	
	COut << "Creating list" << EndLine;
	List<int, 10>	IntList;
	for (int i = 21; i < 36; i++)
	{
		COut << "Push " << i << " to the list... ";
		if (IntList.PushBack(i)) {
			COut << "Ok." << EndLine;
		} else {
			COut << "fail." << EndLine;
		}
	}
	COut << EndLine;
	
	COut << "Listing the list :" << EndLine;
	List<int, 10>::Iterator il = IntList.Begin();
	n = 0;
	while (il != IntList.End())
	{
		COut << " int " << n++ << " is " << *il << EndLine;
		il++;
	}
	COut << "List count = " << IntList.Count() << EndLine;
	COut << EndLine;
	
	COut << "Remove int 23 from the list... ";
	if (IntList.Remove(23)) {
		COut << "Ok" << EndLine;
	} else {
		COut << "Fail" << EndLine;
	}
	COut << "Remove int 27 from the list... ";
	if (IntList.Remove(27)) {
		COut << "Ok" << EndLine;
	} else {
		COut << "Fail" << EndLine;
	}
	COut << "Listing the list : " << EndLine;
	il = IntList.Begin();
	n = 0;
	while (il != IntList.End())
	{
		COut << " int " << n++ << " is " << *il << EndLine;
		il++;
	}
	COut << "List count = " << IntList.Count() << EndLine;
	COut << EndLine;
	
	COut << "Clear the list." << EndLine;
	IntList.Clear();
	COut << "List count = " << IntList.Count() << EndLine;
	COut << EndLine;

	COut << "Push 51 to front... ";
	if (IntList.PushFront(51)) {
		COut << "Ok." << EndLine;
	} else {
		COut << "Fail." << EndLine;
	}
	COut << "Push 65 to front... ";
	if (IntList.PushFront(65)) {
		COut << "Ok." << EndLine;
	} else {
		COut << "Fail." << EndLine;
	}
	COut << "Push 75 to back... ";
	if (IntList.PushBack(75)) {
		COut << "Ok." << EndLine;
	} else {
		COut << "Fail." << EndLine;
	}
	COut << "Push 63 to front... ";
	if (IntList.PushFront(63)) {
		COut << "Ok." << EndLine;
	} else {
		COut << "Fail." << EndLine;
	}
	COut << "Push 39 to back... ";
	if (IntList.PushBack(39)) {
		COut << "Ok." << EndLine;
	} else {
		COut << "Fail." << EndLine;
	}
	COut << "Listing the list : " << EndLine;
	il = IntList.Begin();
	n = 0;
	while (il != IntList.End())
	{
		COut << " int " << n++ << " is " << *il << EndLine;
		il++;
	}
	COut << "List count = " << IntList.Count() << EndLine;
	COut << EndLine;

	COut << "Pop from back... ";
	if (IntList.PopBack(iRes)) {
		COut << "Ok. Result = " << iRes << EndLine;
	} else {
		COut << "Fail." << EndLine;
	}
	COut << "Listing the list : " << EndLine;
	il = IntList.Begin();
	n = 0;
	while (il != IntList.End())
	{
		COut << " int " << n++ << " is " << *il << EndLine;
		il++;
	}
	COut << "List count = " << IntList.Count() << EndLine;
	COut << EndLine;

	COut << "Exit." << EndLine;

	return 0;
}

