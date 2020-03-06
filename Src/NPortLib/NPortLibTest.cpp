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

void __stdcall HandleOnError(const char * szErrorMessage)
{
	cout << "Device error : " << szErrorMessage << endl;
}

int main(int argc, char * argv[])
{
	cout << "Test using NPortLib." << endl;

	int	i;
	i = NPL_Open(HandleOnConnectionChange, HandleOnDataReceived, HandleOnError);
	cout << "Press any key to exit..." << endl;
	
	char c = cin.get();
	if (i == 1) {
		NPL_Close();
	}
	
	return static_cast<int>(c);
}