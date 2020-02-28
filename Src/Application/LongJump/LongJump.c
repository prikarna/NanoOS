/*
 * File    : LongJump.c
 * Remark  : Demonstrate other usage of SetProbe()/ResetProbe() of NanoOS API which is 'simulate' or 'mimic' 
 *           set_jump/long_jump logic. Note that the main purpose of SetProbe()/ResetProbe() is to handle
 *           the fault(s).
 */

#include "..\NanoOSApi.h"

int Add4(int a)
{
	int	i;

	Printf("Pre: %s\r\n", __FUNCTION__);
	i = a + 10;
	Printf("Post: %s\r\n", __FUNCTION__);

	/*
	 * Call ResetProbe() with parameter of TRUE, when success this thread will 'jump back' to where SetProbe()
	 * was called and is about to return. The return value of SetProbe() this time is PROBE_STATUS__RETURN.
	 */
	ResetProbe(TRUE);

	return i;
}

int Mul(int a, int b)
{
	int	i, d;

	Printf("Pre: %s\r\n", __FUNCTION__);
	d = Add4(7);

	/*
	 * If ResetProbe() called from within Add4() success, these lines below will never be executed.
	 */
	i = d * (a + b);
	Printf("Post: %s\r\n", __FUNCTION__);

	return i;
}

int Add2(int a, int b, int c)
{
	int	i, d;

	Printf("Pre: %s\r\n", __FUNCTION__);
	i = a + b - c;
	d = Mul(c, 5);

	/*
	 * If ResetProbe() called from within Add4() success, these lines below will never be executed.
	 */
	i += d;
	Printf("Post: %s\r\n", __FUNCTION__);

	return i;
}

int Add1(int a, int b)
{
	int	i, d;

	Printf("Pre: %s\r\n", __FUNCTION__);
	i = a + b;
	d = Add2(a, b, 3);

	/*
	 * If ResetProbe() called from within Add4() success, these lines below will never be executed.
	 */
	i += d;
	Printf("Post: %s\r\n", __FUNCTION__);

	return i;
}

int main(int argc, char * argv[])
{
	UINT32_T		uProbeStat = 0;
	int				iRes = 0;

	Printf("\r\n***TEST: long jump.\r\n");

	Printf("Set probe... ");
	/* Call SetProbe() */
	uProbeStat = SetProbe();

	/*
	 * When fail just return -1 (exit with return value of -1)
	 */
	if (uProbeStat == PROBE_STATUS__ERROR) {
		Printf("fail! (%d)\r\n", GetLastError());
		return -1;
	}

	/*
	 * When SetProbe() return PROBE_STATUS__RETURN this mean ResetProbe() with parameter TRUE has beed called from 
	 * another part of this thread, in this application ResetProbe() is called from within Add4(), simply return 1 
	 * (exit with return value of 1)
	 */
	if (uProbeStat == PROBE_STATUS__RETURN) {
		Printf("Return from ResetProbe call. Exiting!\r\n");
		return 1;
	}

	/*
	 * Fault(s) may occur in another part of this thread too, if required application can handle this situation by
	 * checking return value of SetProbe(), if this return value is PROBE_STATUS__FAULT do whatever it needs to do to 
	 * handle the fault(s).
	 * Since this application assume that no fault(s) occur, this application do not handle fault(s).
	 */

	/* This lines below executed when SetProbe() success. */
	Printf("success.\r\n");

	/*
	 * Cascaded functions call, this line below call Add1(), Add1() will call Add2(), Add2() will call Mul(), and Mul() 
	 * will call Add4().
	 */
	iRes = Add1(3, 4);

	/*
	 * If ResetProbe() called from within Add4() success, these lines below will never be executed.
	 */
	Printf("3 + 4 = %d\r\n", iRes);

	Printf("Program end.\r\n");

	return 0;
}