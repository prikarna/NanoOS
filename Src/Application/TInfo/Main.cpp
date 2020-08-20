/*
 * File    : Main.cpp
 * Remark  : Demonstrate how to implement and use RTTI in NanoOS.
 *           Ensure no -fno-rtti compiler option in Makefile
 */

#include <typeinfo>
#include "..\NanoStd.h"

using namespace NanoStd;

class MyClass1
{
};

class MyClass2
{
};

class Parent
{
public:
	virtual bool BrownEyes() { return true; }	// Parent is polymorphic
};

class Child1: public Parent
{
};

class Child2: public Parent
{
public:
	bool BrownEyes() { return false; }
};

template <class T> 
class MyTemplate 
{
	T a;
public:
	MyTemplate(T i) { a = i; }
};

class Base1
{
public:
	int	m_iB1Param;
	virtual void Out() { COut << "Is in Base1\r\n"; }
};

class Derived1 : public Base1
{
public:
	void Out() { COut << "Is in Derived1\r\n"; }
};

class DerivedDerived1 : public Derived1 
{
public:
	void Out() { COut << "Is in DerivedDerived1\r\n"; }
};

class Base2
{
public:
	int m_iB2Param;
	virtual void Display() { COut << "Display: Is inside Base2\r\n"; }
};

class Base3
{
public:
	int m_iB3Param;
	virtual void Show() { COut << "Show: Is inside Base3\r\n"; }
};

class MultiDerived1: public Base1, public Base2, public Base3
{
public:
	void Print() { COut << "Print: Is inside MultiDerived1\r\n"; }
	void Out() { COut << "Out: Is inside MultiDerived1\r\n"; }
};

int main(int , char *[])
{
	COut << "Test Run Time Type Information." << EndLine;

	/*
	 * Case 1: Builtin types and regular class
	 */
	int i, j;
	float f;
	char *pChar;
	MyClass1 ObjOne;
	MyClass2 ObjTwo;

	COut << "The type of i is: " << typeid(i).name();
	COut << EndLine;
	COut << "The type of j is: " << typeid(j).name();
	COut << EndLine;
	COut << "The type of f is: " << typeid(f).name();
	COut << EndLine;
	COut << "The type of p is: " << typeid(pChar).name();
	COut << EndLine;

	COut << "The type of ObjOne is: " << typeid(ObjOne).name();
	COut << EndLine;
	COut << "The type of ObjTwo is: " << typeid(ObjTwo).name();
	COut << EndLine;

	if(typeid(i) != typeid(j))
		COut << "The types of i and j are not the same." << EndLine;
	else
		COut << "The types of i and j are the same." << EndLine;
	
	if(typeid(i) == typeid(f))
		COut << "The types of i and f are the same." << EndLine;
	else
		COut << "The types of i and f are not the same." << EndLine;

	if(typeid(ObjOne) != typeid(ObjTwo)) {
		COut << "ObjOne and ObjTwo are of differing types." << EndLine;
	}
	COut << EndLine;

	/*
	 * Case 2: Inheritance class (polymorphic class)
	 */
	Parent *p, Par;
	Child1 ChildOne;
	Child2 ChildTwo;

	p = &Par;
	COut << "p is pointing to an object of type " << typeid(*p).name() << EndLine;
	p = &ChildOne;
	COut << "p is pointing to an object of type " << typeid(*p).name() << EndLine;
	p = &ChildTwo;
	COut << "p is pointing to an object of type " << typeid(*p).name() << EndLine;
	COut << EndLine;
	
	/*
	 * Case 3: Class template
	 */
	MyTemplate<int>	ITemp1(10), ITemp2(20);
	MyTemplate<double> DTemp(10.10);

	COut << "Type of ITemp1 is " << typeid(ITemp1).name() << EndLine;
	COut << "Type of ITemp2 is " << typeid(ITemp2).name() << EndLine;
	COut << "Type of DTemp is " << typeid(DTemp).name() << EndLine;
	if (typeid(ITemp1) == typeid(ITemp2)) {
		COut << "ITemp1 and ITemp2 are the same types." << EndLine;
	} else {
		COut << "ITemp1 and ITemp2 are different types." << EndLine;
	}
	if (typeid(ITemp1) == typeid(DTemp)) {
		COut << "ITemp1 and DTemp are the same types." << EndLine;
	} else {
		COut << "ITemp1 and DTemp are different types." << EndLine;
	}
	COut << EndLine;

	/*
	 * Case 4: Derived class
	 */
	Base1			*pBaseOne, BaseOneObj;
	Derived1		*pDrvOne, DrvOneObj;
	DerivedDerived1	*pDDrvOne, DDrvOneObj;

	pBaseOne = dynamic_cast<Base1 *> (&DrvOneObj);
	if (pBaseOne) {
		COut << "Cast from Derived1 * to Base1 * is OK.\r\n";
		pBaseOne->Out();
	} else {
		COut << "Error!\r\n";
	}
	COut << EndLine;

	pBaseOne = dynamic_cast<Base1 *> (&BaseOneObj);
	if (pBaseOne) {
		COut << "Cast from Base1 * to Base1 * is OK.\r\n";
		pBaseOne->Out();
	} else {
		COut << "Error!\r\n";
	}
	COut << EndLine;

	pDrvOne = dynamic_cast<Derived1 *> (&BaseOneObj);	// May generate a warning at compile time
	if (pDrvOne) {
		COut << "Error!\r\n";
	} else {
		COut << "Cast from Base1 * to Derived1 * is NOT OK.\r\n";
	}
	COut << EndLine;

	pBaseOne = &DrvOneObj;		// pBaseOne now point ot DrvOneObj
	pDrvOne = dynamic_cast<Derived1 *> (pBaseOne);	// require __dynamic_cast()
	if (pDrvOne) {
		COut << "Casting Base1 * to a Derived1 * is OK because pBaseOne is really pointing to a Derived1 object.\r\n";
		pDrvOne->Out();
	} else {
		COut << "Error!\r\n";
	}
	COut << EndLine;

	pBaseOne = &BaseOneObj;		// pBaseOne now point to BaseOneObj
	pDrvOne = dynamic_cast<Derived1 *> (pBaseOne);	// require __dynamic_cast()
	if (pDrvOne)
		COut << "Error!\r\n";
	else {
		COut << "Now casting Base1 * to a Derived1 * is NOT OK because pBaseObj is really pointing to a Base1 object.\r\n";
	}
	COut << EndLine;

	pDrvOne = &DrvOneObj;	// pDrvOne points to Derived1 object
	pBaseOne = dynamic_cast<Base1 *> (pDrvOne);
	if (pBaseOne) {
		COut << "Casting pDrvOne to a Base * is OK.\r\n";
		pBaseOne->Out();
	} else {
		COut << "Error!\r\n";
	}
	COut << EndLine;

	pBaseOne = &DDrvOneObj;
	pDDrvOne = dynamic_cast<DerivedDerived1 *>(pBaseOne);	// require __dynamic_cast()
	if (pDDrvOne) {
		COut << "Casting pBaseOne to a DerivedDerived1 * is OK because pBaseOne is "
			<< "really pointing to a DerivedDerived1 object.\r\n";
		pDDrvOne->Out();
	} else {
		COut << "Error!\r\n";
	}
	COut << EndLine;

	/*
	 * Case 5: Multiple inheritance class
	 */
	Base3 *pBaseThree;
	MultiDerived1 *pMDrvOne, MDrvOneObj;
	
	pBaseThree = 0;
	MDrvOneObj.m_iB1Param = 100;	// m_iB1Param is inherited from Base1 class
	MDrvOneObj.m_iB2Param = 200;	// m_iB2Param is inherited from Base2 class
	MDrvOneObj.m_iB3Param = 300;	// m_iB3Param is inherited from Base3 class
	
	pBaseThree = &MDrvOneObj;
	pMDrvOne = dynamic_cast<MultiDerived1 *>(pBaseThree);	// require __dynamic_cast()
	if (pMDrvOne) {
		COut << "Casting pBaseThree to MultiDerived1 * is OK." << EndLine;
		COut << "    m_iB1Param = " << pMDrvOne->m_iB1Param << EndLine
			 << "    m_iB2Param = " << pMDrvOne->m_iB2Param << EndLine 
			 << "    m_iB3Param = " << pMDrvOne->m_iB3Param << EndLine; 
		pMDrvOne->Show();
		pMDrvOne->Print();
		pMDrvOne->Out();
		pMDrvOne->Display();
	} else {
		COut << "Casting pBaseThree to MultiDerived * is NOT OK\r\n";
	}
	COut << EndLine;

	return 0;
}
