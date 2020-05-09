/*
 * File		: Main.c
 * Desc.	:
 *		This is the simple sample of hello world program, using Printf function
 *		of NanoOS API.
 *		Note: NanoOS currently do not have standard C library
 */

#include "NanoOSApi.h"

int main(int argc, char * argv[])
{
	Printf("Hello world.\r\n", 0);
	return 0;
}
