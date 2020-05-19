/*
 * File    : Main.cpp
 * Remark  : Demonstrate how to write NanoOS application in C++ using NanoStd namespace.
 */

#include "NanoStd.h"

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

	SArray<char, 64>	szBuf;
	COut << "Tell me your name: ";
	CIn >> szBuf;
	COut << "Hello " << szBuf << '.' << EndLine;

	COut << "Exit." << EndLine;

	return 0;
}

