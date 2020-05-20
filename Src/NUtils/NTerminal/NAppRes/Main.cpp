/*
 * File    : Main.cpp
 * Desc.	:
 *		This is the simple sample of hello world program, using NanoStd (basic input output 
 *		specifically for NanoOS)
 *		Note: NanoOS currently do not have standard C library and standard C++ class
 */

#include "NanoStd.h"

using namespace NanoStd;

class Test
{
public:
	Test() { COut << __FUNCTION__ << " c'tor." << EndLine; }
	~Test() { COut << __FUNCTION__ << " d'tor." << EndLine; }
};

Test t1;

int main(int , char *[])
{
	COut << "Hello world." << EndLine;
	return 0;
}
