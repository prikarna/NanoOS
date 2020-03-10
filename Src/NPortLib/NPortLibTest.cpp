/*
 * File    : NPortLibTest.cpp
 * Remark  : Demonstrate how to use NPortLib library. This is compiled as C++ code but still using 
 *           C functions in NPortLib.h.
 */

#include <iostream>
#include <string>

#include "NPortLib.h"

using namespace std;

void __stdcall HandleOnConnectionChange(unsigned char fIsConnected)
{
	if (fIsConnected == 0) {
		cout << "*** Device is disconnected!" << endl;
	} else {
		cout << "*** Device is connected" << endl;
	}
}

void __stdcall HandleOnDataReceived(unsigned char * pBuff, int iLength)
{
	cout << "Data received, length = " << iLength << endl;
	cout << "Content: " << reinterpret_cast<char *>(pBuff);
}

void __stdcall HandleOnError(const char * szErrorMessage, unsigned long ulErrorCode)
{
	cout << "Error " << ulErrorCode << ": " << szErrorMessage << endl;
}

void __stdcall HandleOnInstall(int iProgress)
{
	if (iProgress < 100) {
		cout << "#";
	} else {
		cout << " Completed." << endl;
	}
}

int main(int argc, char * argv[])
{
	cout << "Test using NPortLib." << endl;

	int	i;
	i = NPL_Open(HandleOnConnectionChange, HandleOnDataReceived, HandleOnError);
	if (i == 1) {
		if (NPL_IsOpen() == 1) {
			string str;
			cout << "NanoOS application file to install: ";
			cin >> str;
			cout << "Install file " << str;
			i = NPL_InstallNApplication(str.c_str(), HandleOnInstall);
		}
	}

	system("PAUSE");

	if (NPL_IsOpen() == 1) {
		NPL_Close();
	}

	return 0;
}