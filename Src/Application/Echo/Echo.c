/*
 * File    : Echo.c
 * Remark  : Echo back to PC application the last data writen (or sent) with the same length, this
 *			 mean when PC application write 24 bytes data, then this NanoOS application will write 
 *			 back with received (read) data and with the length of 24 bytes. See NPortSvcTest project 
 *			 for more detail in the code; how to write and read data with this NanoOS application. In
 *			 NPortSvcTest, if it write 62 bytes data then it should read with the buffer length of 62
 *			 bytes data.
 *
 */

#include "..\NanoOSApi.h"

void TerminateApp(UINT32_T uReason)
{
	DBG_PRINTF("%s\r\n", __FUNCTION__);
	CancelReadFromUsbSerial();
}

int main(int argc, char * argv[])
{
	UINT8_T		szBuf[64];
	BOOL		fRes = FALSE;
	UINT32_T	uiReadLen = 0;

	fRes = SetTerminationHandler(TerminateApp);
	if (!fRes) {
		return -1;
	}

	while (TRUE) {
		uiReadLen = 0;
		MemSet(szBuf, 0, sizeof(szBuf));
		fRes = ReadFromUsbSerial(szBuf, (sizeof(szBuf) - 2), &uiReadLen);
		if (fRes) {
			fRes = WriteToUsbSerial(szBuf, uiReadLen);
		}
	}

	return 0;
}