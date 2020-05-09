/*
 * File    : NPortSvcTest.cpp
 * Remark  : To test NanoOS Port service program (NPortSvc). This test is in conjunction with NanoOS
 *           application so called 'Echo' which just echo what it received from accessing PC program.
 *
 */

#include <cstdio>
#include <iostream>
#include <string>

#include "TChar.h"
#include "..\NPortLib\NPortLib.h"

using namespace std;

#ifdef UNICODE
# define _tcout		std::wcout
# define _tcin		std::wcin
# define _tstring	std::wstring
# define _tcerr		std::wcerr
#else
# define _tcout		std::cout
# define _tcin		std::cin
# define _tstring	std::string
# define _tcerr		std::cerr
#endif

#include "NPortClient.h"

void __stdcall HandleConnectionChanged(int fConnected)
{
	if (fConnected == 1) {
		_tcout << "Device connected." << endl;
	} else {
		_tcout << "Device disconnected." << endl;
	}
}

int _tmain(int argc, TCHAR * argv[])
{
	_tcout << _T("Test NPort service.") << endl;
	_tcout << _T("Press <Enter> key to continue...") << endl;

	_tcout << _T("Open port client... ");
	int iRes = NPLSVC_Open(HandleConnectionChanged);
	if (iRes == 0) {
		_tcout << _T("failed! (") << NPLSVC_GetLastError() << _T(").") << endl;
		int i = _tcin.get();
		return -1;
	} else {
		_tcout << _T("success.") << endl;

		/*
		 * To test in 'interactive program'
		 */
		_tcout << _T("Type 'quit' then press <Enter> to quit this application!") << endl;
		while (true) {
			char	szBuf[62];
			
			cin.getline(szBuf, sizeof(szBuf));
			if (_strcmpi(&szBuf[0], "quit") == 0) {
				break;
			}

			if (strlen(szBuf) == 0) continue;

			iRes = NPLSVC_Write((const unsigned char *)&(szBuf[0]), sizeof(szBuf));
			if (iRes == 1) {
				int iRead = 0;
				memset((void *) szBuf, 0, sizeof(szBuf));
				iRes = NPLSVC_Read((unsigned char *) &szBuf[0], sizeof(szBuf), &iRead);
				if (iRes == 1) {
					_tcout << szBuf << endl;
				} else {
					_tcout << "Error read! (" << NPLSVC_GetLastError() << ")." << endl;
				}
			} else {
				_tcout << "Error write! (" << NPLSVC_GetLastError() << ")." << endl;
			}
		}

		/*
		 * To test NPortSvc for write-read pair continuously.
		 */
		//int iNumb = 0;
		//while (true) {
		//	char	szBuf[62];
		//	
		//	memset((void *) szBuf, 0, sizeof(szBuf));
		//	_snprintf_s(szBuf, sizeof(szBuf), sizeof(szBuf) - 1, "String number %d", iNumb++);
		//	iRes = NPLSVC_Write((const unsigned char *)&(szBuf[0]), sizeof(szBuf));
		//	if (iRes == 1) {
		//		int iRead = 0;
		//		memset((void *) szBuf, 0, sizeof(szBuf));
		//		iRes = NPLSVC_Read((unsigned char *) &szBuf[0], (unsigned int) sizeof(szBuf), &iRead);
		//		if (iRes) {
		//			_tcout << szBuf << endl;
		//		} else {
		//			_tcout << "Error read! (" << NPLSVC_GetLastError() << ")." << endl;
		//			iNumb--;
		//		}
		//	} else {
		//		_tcout << "Error write! (" << NPLSVC_GetLastError() << ")." << endl;
		//		iNumb--;
		//	}
		//}

		/*
		 * To test write-read operation in a 'single request session' continuously.
		 */
		//int iNumb = 0;
		//while (TRUE) 
		//{
		//	char	szWrBuf[62];
		//	char	szRdBuf[62];
		//	int		iReadLen = 0;

		//	memset(szWrBuf, 0, sizeof(szWrBuf));
		//	memset(szRdBuf, 0, sizeof(szRdBuf));

		//	_snprintf_s(szWrBuf, sizeof(szWrBuf), sizeof(szWrBuf) - 1, "String number %d", iNumb++);
		//	
		//	iRes = NPLSVC_WriteThenRead(
		//					(const unsigned char *) szWrBuf, 
		//					sizeof(szWrBuf), 
		//					(unsigned char *) szRdBuf, 
		//					sizeof(szRdBuf), 
		//					&iReadLen
		//					);
		//	if (iRes) {
		//		_tcout << szRdBuf << endl;
		//	} else {
		//		_tcout << "Error Write-Read! (" << NPLSVC_GetLastError() << ")." << endl;
		//		iNumb--;
		//	}
		//}
	}

	NPLSVC_Close();

	return 0;
}