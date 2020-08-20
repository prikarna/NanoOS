/*
 * File    : Main.cpp
 * Remark  : Demonstrate how to implement and use C++ exception in NanoOS.
 *           Ensure the compiler don't specify -fno-rtti and -fno-exceptions options in Makefile
 *           Note: The exception implementation has some limitation such as it can't be used for
 *                 multiple try-catch in one function.
 */

#include <typeinfo>
#include "..\NanoStd.h"

using namespace NanoStd;

void Case2Exception(int iParm)
{
	COut << "Inside Case2Exception, iParm is: " << iParm << EndLine;
	if (iParm) throw iParm;
}

/*
 * Case 2: Throw exception from another function.
 */
void Case2()
{
	COut << "++ Case 2 ++" << EndLine;
	try {
		COut << "Inside Case2 try block" << EndLine;
		Case2Exception(0);
		Case2Exception(1);
		Case2Exception(2);
	}
	catch (int i) {
		COut << "Caught an exception -- value is: ";
		COut << i << EndLine;
	}
	COut << EndLine;
}

class NException 
{
public:
	char Message[64];
	int Param;

	NException() 
	{ 
		*Message = 0; 
		Param = 0; 
	}

	NException(const char *s, int e) 
	{
		StrCopy(Message, s, sizeof(Message));
		Param = e;
	}
};

/*
 * Case 3: Throw exception object.
 */
void Case3()
{
	int i;
	COut << "++ Case 3 ++" << EndLine;
	try {
		COut << "Enter a positive number: ";
		CIn >> i;
		if (i < 0)
			throw NException("Not Positive", i);

		COut << "Your number is: " << i;
	}
	catch (NException e) { // catch an error
		COut << e.Message << ": ";
		COut << e.Param << EndLine;
	}
	COut << EndLine;
}

void Case4Exception(int iParm)
{
	try{
		if (iParm == 0) {
			throw "Value is zero";
		} else {
			if (iParm == 2) {
				throw iParm;
			}

			if (iParm > 100) {
				throw NException("More than 100", iParm);
			}

			COut << "Case4Exception: iParm = " << iParm << EndLine;
		}
	}
	catch(int i) {
		COut << "Caught Exception #: " << i << EndLine;
	}
	catch (NException e) { // catch an error
		COut << "Caught NException: ";
		COut << e.Message << ": ";
		COut << e.Param << EndLine;
	}
	catch(const char *str) {
		COut << "Caught a string: ";
		COut << str << EndLine;
	}
}

/*
 * Case 4: Throw multiple exception and catch them.
 */
void Case4()
{
	COut << "++ Case 4 ++" << EndLine;
	Case4Exception(1);
	Case4Exception(2);
	Case4Exception(0);
	Case4Exception(3);
	Case4Exception(101);
	COut << EndLine;
}

class CustException: public NException
{
public:
	CustException() {}
	
	CustException(const char *szMessage, int iParam):
	NException(szMessage, iParam)
	{
	}
};

/*
 * Case 5: Throw derived exception object.
 */
void Case5()
{
	CustException CustExc;
	COut << "++ Case 5 ++" << EndLine;
	try {
		throw CustExc;
	}
	//catch(CustException c) {
	//	COut << "Caught a derived class.\r\n";
	//}
	catch(NException m) {
		COut << "Caught a base class.\r\n";
	}
	/*
	 * This handler will not be caught and compiler will warn about this. 
	 * To resolve this problem comment this handler and uncomment CustException
	 * handler above NException handler.
	 */
	catch(CustException c) { 
		COut << "Caught a derived class.\r\n";
	}
	COut << EndLine;
}

void Case6Exception(int iParm)
{
	try{
		if(iParm==0) throw iParm; // throw int.
		if(iParm==1) throw 'a'; // throw char.
		if(iParm==2) throw 123.23; // throw double.
	}
	catch(int i) { // catch an int exception.
		COut << "Caught an integer\r\n";
	}
	catch(...) { // catch all other exceptions.
		COut << "Caught One!\r\n";
	}
}

/*
 * Case 6: Default handler.
 */
void Case6()
{
	COut << "++ Case 6 ++" << EndLine;
	Case6Exception(0);
	Case6Exception(1);
	Case6Exception(2);
	COut << EndLine;
}

void Case7Exception(int iParm) throw(int, char, double)
/*
 * Uncomment the line below and comment the line above
 * to test exception restriction, the line below will allow
 * only int and char exception. When exception mechanism
 * encounter a restriction, it will terminate current thread.
 */
//void Case7Exception(int iParm) throw(int, char)
{
	if(iParm==0) throw iParm; // throw int.
	if(iParm==1) throw 'c'; // throw char.
	if(iParm==2) throw 888.88; // throw double.
}

/*
 * Case 7: Exception restriction.
 */
void Case7()
{
	COut << "++ Case 7 ++" << EndLine;
	try{
		Case7Exception(2); // also, try passing 0 and 1 to Case7Exception().
	}
	catch(int i) {
		COut << "Caught an integer\r\n";
	}
	catch(char c) {
		COut << "Caught char\r\n";
	}
	catch(double d) {
		COut << "Caught double\r\n";
	}
	COut << EndLine;
}

void Case8Exception()
{
	try {
		throw "hello"; // throw a char *
	}
	catch(const char *) { // catch a char *
		COut << "Caught char * inside Case8Exception\r\n";
		throw ; // rethrow char * out of function.
	}
}

/*
 * Case 8: Re-throw exception.
 */
void Case8()
{
	COut << "++ Case 8 ++" << EndLine;
	try{
		Case8Exception();
	}
	catch(const char *) {
		COut << "Caught char * inside Case8\r\n";
	}
	COut << EndLine;
}

void Case9Exception()
{
	UINT32_PTR_T	pFaultAddr = (UINT32_PTR_T) 0x1FFFFFFC;
	UINT32_T	u;

	COut << "Try to access invalid memory... " << EndLine;
	u = *pFaultAddr;

	COut << "Value of u is " << u << EndLine;
	ResetProbe(FALSE);
}

class SysException
{
public:
	char Message[64];
	unsigned int Fault;
	unsigned int BusAddress;
	unsigned int MemAddress;
	SysException(const char *szMessage, unsigned int uFault, unsigned int uBusAddress, unsigned int uMemAddress)
	{
		StrCopy(Message, szMessage, sizeof(Message));
		Fault = uFault;
		BusAddress = uBusAddress;
		MemAddress = uMemAddress;
	}
};

/*
 * Case 9: Catch system fault, in cooperation with SetProbe().
 */
void Case9()
{
	UINT32_T	uProbeStat;
	UINT32_T	uFault;
	UINT32_T	uBusAddr;
	UINT32_T	uMemAddr;

	COut << "++ Case 9 ++" << EndLine;
	try {
		uProbeStat = SetProbe();	// Should call SetProbe() first
		if (uProbeStat == PROBE_STATUS__FAULT) {
			uFault = GetLastFault();
			GetFaultAddress(uFault, &uBusAddr, &uMemAddr);
			throw SysException("Faulty Access", uFault, uBusAddr, uMemAddr);
		}

		Case9Exception();
	}
	catch (SysException& se)
	{
		COut << "Caught system fault : " << se.Message << EndLine;
		COut << "    Fault Code          : 0x" << Hexa << se.Fault << EndLine
			 << "    Faulty bus Address  : 0x" << se.BusAddress << EndLine
			 << "    Faulty mem. Address : 0x" << se.MemAddress << EndLine;
		COut << Decimal;
	}
	COut << EndLine;
}

int main(int , char *[])
{
	COut << "Test Exceptions." << EndLine;

	/*
	 * Case 1: Simple exception
	 */
	COut << "++ Case 1 ++" << EndLine;
	try {
		COut << "Before throw an exception." << EndLine;
		throw 100;
		COut << "This line should never be executed." << EndLine;
	}
	catch (int i) {
		COut << "Exception int occured!, i = " << i << EndLine;
	}
	COut << EndLine;

	Case2();

	Case3();

	Case4();

	Case5();

	Case6();

	Case7();

	Case8();

	Case9();

	return 0;
}
