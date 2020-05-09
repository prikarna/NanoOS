#include "NAppGen.h"

NAppGen::NAppGen():
m_pMainFileRes(NULL),
m_dwMainFileSize(0),
m_pSrcIncFile(NULL),
m_dwSrcIncSize(0),
m_bIncSolution(false)
{
	RtlZeroMemory(m_szSlnName, sizeof(m_szSlnName));
	RtlZeroMemory(m_szProjName, sizeof(m_szProjName));
	RtlZeroMemory(m_szIncPath, sizeof(m_szIncPath));
	RtlZeroMemory(m_szGccPath, sizeof(m_szGccPath));
}

NAppGen::~NAppGen()
{
	CoUninitialize();
}

bool NAppGen::_LoadResource(HMODULE hMod, WORD wResId, WORD wResType, LPVOID *ppRetRes, LPDWORD pdwResSize)
{
	if ((!ppRetRes) || (!pdwResSize)) {
		m_dwError = ERROR_INVALID_PARAMETER;
		return false;
	}

	HRSRC hRes = FindResource(hMod, MAKEINTRESOURCE(wResId), MAKEINTRESOURCE(wResType));
	if (!hRes) {
		m_dwError = GetLastError();
		return false;
	}

	HGLOBAL	hGlob = LoadResource(hMod, hRes);
	if (!hGlob) {
		m_dwError = GetLastError();
		return false;
	}

	*ppRetRes = LockResource(hGlob);
	if (!(*ppRetRes)) {
		m_dwError = GetLastError();
		return false;
	}

	*pdwResSize = SizeofResource(hMod, hRes);
	if (*pdwResSize == 0) {
		m_dwError = GetLastError();
		return false;
	}

	return true;
}

bool NAppGen::Initialize()
{
	CoInitialize(NULL);

	HMODULE	hMod = GetModuleHandle(NULL);
	bool bRes = _LoadResource(hMod, DKF_MAIN_RES_ID, DKF_MAIN_RES_TYPE, &m_pMainFileRes, &m_dwMainFileSize);
	if (bRes) {
		bRes = _LoadResource(hMod, DKF_SRC_INC_RES_ID, DKF_SRC_INC_RES_TYPE, &m_pSrcIncFile, &m_dwSrcIncSize);
	}

	return bRes;
}

bool NAppGen::_CreateSolution(bool fOverwrite)
{
	HRESULT hres = S_OK;

	RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
	hres = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s\\%s.sln", m_szProjPath, m_szSlnName);
	if (hres != S_OK) {
		m_dwError = HRESULT_CODE(hres);
		return false;
	}

	DWORD	dwDisp;
	if (fOverwrite) {
		dwDisp = CREATE_ALWAYS;
	} else {
		dwDisp = OPEN_ALWAYS;
	}

	HANDLE	hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFileA(
				m_szBuff,
				GENERIC_ALL,
				0,
				NULL,
				dwDisp,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);
	if (hFile == INVALID_HANDLE_VALUE) {
		m_dwError = GetLastError();
		return false;
	}

	BOOL fRes = FALSE;
	DWORD dwWr = 0;
	PCH	pszLine = NULL;

	for (UINT uiResId = DKS_SLN_BEGIN; uiResId < DKS_SLN_END; uiResId++)
	{
		pszLine = const_cast<CHAR *>(m_szSln[uiResId]);

		m_dwError = 0;
		switch (uiResId)
		{
		case DKS_SLN_PROJ_GUID_NAME:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hres = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s\"%s\", \"%s\\%s.vcproj\", \"{%s}\"\n",
				pszLine, m_szProjName, m_szProjName, m_szProjName, m_pszProjUUID);
			if (hres != S_OK) {
				m_dwError = HRESULT_CODE(hres);
				fRes = FALSE;
			} else {
				fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			}
			break;

		case DKS_SLN_DBG_ACT_CFG:
		case DKS_SLN_REL_ACT_CFG:
		case DKS_SLN_REL_BLD_0:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hres = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "\t\t{%s}%s", m_pszProjUUID, pszLine);
			if (hres != S_OK) {
				m_dwError = HRESULT_CODE(hres);
				fRes = FALSE;
			} else {
				fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			}
			break;

		default:
			fRes = WriteFile(hFile, (LPCVOID) pszLine, lstrlenA(pszLine), &dwWr, NULL);
			break;
		}

		if (!fRes) {
			if (m_dwError == 0)
				m_dwError = GetLastError();
			break;
		}
	}

	CloseHandle(hFile);

	return ((fRes) ? true : false);
}

bool NAppGen::_CreateProject(bool fOverwrite)
{
	HRESULT hr = S_OK;
	BOOL	fRes = FALSE;

	if (m_bIncSolution) {
		RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
		hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s\\%s", m_szProjPath, m_szProjName);
		if (hr != S_OK) {
			m_dwError = HRESULT_CODE(hr);
			return false;
		}

		fRes = CreateDirectoryA(m_szBuff, NULL);
		if (!fRes) {
			m_dwError = GetLastError();
			if (m_dwError != ERROR_ALREADY_EXISTS) {
				return false;
			}
		}

		RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
		hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s\\%s\\%s.vcproj", m_szProjPath, m_szProjName, m_szProjName);
		if (hr != S_OK) {
			m_dwError = HRESULT_CODE(hr);
			return false;
		}
	} else {
		RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
		hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s\\%s.vcproj", m_szProjPath, m_szProjName);
		if (hr != S_OK) {
			m_dwError = HRESULT_CODE(hr);
			return false;
		}
	}

	DWORD	dwDisp;
	if (fOverwrite) {
		dwDisp = CREATE_ALWAYS;
	} else {
		dwDisp = OPEN_ALWAYS;
	}

	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFileA(
				m_szBuff,
				GENERIC_ALL,
				0,
				NULL,
				dwDisp,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);
	if (hFile == INVALID_HANDLE_VALUE) {
		m_dwError = GetLastError();
		return false;
	}

	DWORD	dwWr = 0;
	PCH		pszLine = NULL;
	
	fRes = FALSE;

	for (UINT uiResId = DKS_PROJ_BEGIN; uiResId < DKS_PROJ_END; uiResId++)
	{
		pszLine = const_cast<CHAR *>(m_szProj[uiResId]);

		m_dwError = 0;
		switch (uiResId)
		{
		case DKS_PROJ_NAME:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s\"%s\"\n", pszLine, m_szProjName);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				fRes = FALSE;
				break;
			}
			fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			break;

		case DKS_PROJ_GUID:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s\"{%s}\"\n", pszLine, m_pszProjUUID);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				fRes = FALSE;
				break;
			}
			fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			break;

		case DKS_PROJ_DBG_OUTPUT:
		case DKS_PROJ_REL_OUTPUT:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s\"%s.bin\"\n", pszLine, m_szProjName);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				fRes = FALSE;
				break;
			}
			fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			break;

		case DKS_PROJ_DBG_INC_PATH:
		case DKS_PROJ_REL_INC_PATH:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s\"%s\\Application\"\n", pszLine, m_szIncPath);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				fRes = FALSE;
				break;
			}
			fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			break;

		default:
			fRes = WriteFile(hFile, (LPCVOID) pszLine, lstrlenA(pszLine), &dwWr, NULL);
			break;
		}

		if (!fRes) {
			if (m_dwError == 0)
				m_dwError = GetLastError();
			break;
		}
	}

	CloseHandle(hFile);

	return ((fRes) ? true : false);
}

bool NAppGen::_CreateMakefile(bool fOverwrite)
{
	HRESULT hr = S_OK;
	BOOL	fRes = FALSE;

	if (m_bIncSolution) {
		RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
		hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s\\%s\\Makefile", m_szProjPath, m_szProjName);
		if (hr != S_OK) {
			m_dwError = HRESULT_CODE(hr);
			return false;
		}
	} else {
		RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
		hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s\\Makefile", m_szProjPath);
		if (hr != S_OK) {
			m_dwError = HRESULT_CODE(hr);
			return false;
		}
	}

	DWORD	dwDisp;
	if (fOverwrite) {
		dwDisp = CREATE_ALWAYS;
	} else {
		dwDisp = OPEN_ALWAYS;
	}

	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFileA(
				m_szBuff,
				GENERIC_ALL,
				0,
				NULL,
				dwDisp,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);
	if (hFile == INVALID_HANDLE_VALUE) {
		m_dwError = GetLastError();
		return false;
	}

	DWORD	dwWr = 0;
	PCH		pszLine = NULL;

	fRes = FALSE;

	for (UINT uiResId = DKS_MKF_BEGIN; uiResId < DKS_MKF_END; uiResId++)
	{
		pszLine = const_cast<CHAR *>(m_szMkfile[uiResId]);

		m_dwError = 0;
		switch (uiResId)
		{
		case DKS_MKF_OUT_DIR:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s%s\\Bin\n", pszLine, m_szProjPath);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				fRes = FALSE;
				break;
			}
			fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			break;

		case DKS_MKF_SRCS_DIR:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			if (m_bIncSolution) {
				hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s%s\\%s\n", pszLine, m_szProjPath, m_szProjName);
			} else {
				hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s%s\n", pszLine, m_szProjPath);
			}
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				fRes = FALSE;
				break;
			}
			fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			break;

		case DKS_MKF_INCLUDE:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s%s\\Application\n", pszLine, m_szIncPath);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				fRes = FALSE;
				break;
			}
			fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			break;

		case DKS_MKF_GCC_DIR:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s%s\n", pszLine, m_szGccPath);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				fRes = FALSE;
				break;
			}
			fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			break;

		case DKS_MKF_TGT_NAME:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s%s\n", pszLine, m_szProjName);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				fRes = FALSE;
			}
			fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			break;

		default:
			fRes = WriteFile(hFile, (LPCVOID) pszLine, lstrlenA(pszLine), &dwWr, NULL);
			break;
		}

		if (!fRes) {
			if (m_dwError == 0)
				m_dwError = GetLastError();
			break;
		}
	}

	CloseHandle(hFile);

	return ((fRes) ? true : false);
}

bool NAppGen::_CreateFile(bool fOverwrite, const char *szFileName, WORD wResId, WORD wResType)
{
	HRESULT	hr = S_OK;

	if (!szFileName) {
		m_dwError = ERROR_INVALID_PARAMETER;
		return false;
	}

	RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
	if (m_bIncSolution) {
		hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s\\%s\\%s", m_szProjPath, m_szProjName, szFileName);
		if (hr != S_OK) {
			m_dwError = HRESULT_CODE(hr);
			return false;
		}
	} else {
		hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s\\%s", m_szProjPath, szFileName);
		if (hr != S_OK) {
			m_dwError = HRESULT_CODE(hr);
			return false;
		}
	}

	DWORD	dwDisp = 0;
	if (fOverwrite) {
		dwDisp = CREATE_ALWAYS;
	} else {
		dwDisp = OPEN_ALWAYS;
	}

	HANDLE	hFile = INVALID_HANDLE_VALUE;
	BOOL	fRes = FALSE;
	bool	bRes = false;
	DWORD	dwWr = 0;

	m_dwError = ERROR_SUCCESS;

	do {
		hFile = CreateFileA(
					m_szBuff,
					GENERIC_ALL,
					0,
					NULL,
					dwDisp,
					FILE_ATTRIBUTE_NORMAL,
					NULL
					);
		if (hFile == INVALID_HANDLE_VALUE) {
			m_dwError = GetLastError();
			break;
		}

		if (lstrcmpiA(szFileName, "Main.c") == 0)
		{
			fRes = WriteFile(hFile, m_pMainFileRes, m_dwMainFileSize, &dwWr, NULL);
		}
		else if (lstrcmpiA(szFileName, "Sources.inc") == 0)
		{
			fRes = WriteFile(hFile, m_pSrcIncFile, m_dwSrcIncSize, &dwWr, NULL);
		}
		else
		{
			m_dwError = ERROR_INVALID_PARAMETER;
			fRes = FALSE;
		}

	} while (FALSE);

	if (!fRes) {
		if (m_dwError != ERROR_SUCCESS)
			m_dwError = GetLastError();
	}

	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	return ((fRes) ? true : false);
}

INT_PTR CALLBACK NAppGen::_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParm, LPARAM lParm)
{
	HWND	hCh;
	LRESULT	lRes;
	BOOL	fRes = FALSE;
	static	NAppGen *		pThis = NULL;
	static	BROWSEINFOA		bi = {0};
	PIDLIST_ABSOLUTE	pShRet = NULL;
	bool	bRes = false;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		pThis = reinterpret_cast<NAppGen *>(lParm);
		bi.hwndOwner = hDlg;
		bi.ulFlags = BIF_RETURNONLYFSDIRS;
		RtlZeroMemory(pThis->m_szProjPath, sizeof(pThis->m_szProjPath));
		RtlZeroMemory(pThis->m_szIncPath, sizeof(pThis->m_szIncPath));
		RtlZeroMemory(pThis->m_szGccPath, sizeof(pThis->m_szGccPath));
		return (INT_PTR)TRUE;

	case WM_SHOWWINDOW:
		hCh = GetDlgItem(hDlg, DKGC_PROJ_NAME);
		if (hCh) SetFocus(hCh);
		SetDlgItemText(hDlg, DKGC_PROJ_NAME, _T("NApplication"));
		SetDlgItemText(hDlg, DKGC_SLN_NAME, _T("NSolution"));
		break;

	case WM_COMMAND:
		switch(LOWORD(wParm))
		{
		case DKGC_BTN_CANCEL:
			EndDialog(hDlg, DKGC_BTN_CANCEL);
			return (INT_PTR)TRUE;

		case DKGC_BTN_OK:
			bRes = pThis->_CheckInput(hDlg);
			if (bRes) {
				EndDialog(hDlg, DKGC_BTN_OK);
			}
			return (INT_PTR)TRUE;

		case DKGC_SLN_CHECK:
			hCh = (HWND) lParm;
			lRes = SendMessage(hCh, BM_GETCHECK, 0, 0);
			if (lRes == BST_CHECKED) {
				fRes = SendMessage(GetDlgItem(hDlg, DKGC_SLN_NAME), EM_SETREADONLY, FALSE, 0);
				SetFocus(GetDlgItem(hDlg, DKGC_SLN_NAME));
				pThis->m_bIncSolution = true;
			} else {
				fRes = SendMessage(GetDlgItem(hDlg, DKGC_SLN_NAME), EM_SETREADONLY, TRUE, 0);;
				pThis->m_bIncSolution = false;
			}
			break;

		case DKGC_PROJ_BROW:
			RtlZeroMemory(pThis->m_szProjPath, sizeof(pThis->m_szProjPath));
			bi.lpszTitle = "Select or create directory for Solution and/or Project";
			bi.ulFlags |= BIF_NEWDIALOGSTYLE;
			pShRet = SHBrowseForFolderA(&bi);
			if (pShRet) {
				fRes = SHGetPathFromIDListA(pShRet, pThis->m_szProjPath);
				if (fRes) {
					SetDlgItemTextA(hDlg, DKGC_PROJ_PATH, pThis->m_szProjPath);
				}
				CoTaskMemFree(pShRet);
			}
			break;

		case DKGC_INC_BROW:
			RtlZeroMemory(pThis->m_szIncPath, sizeof(pThis->m_szIncPath));
			bi.lpszTitle = "Select directory of NanoOS source";
			bi.ulFlags &= ~BIF_NEWDIALOGSTYLE;
			pShRet = SHBrowseForFolderA(&bi);
			if (pShRet) {
				fRes = SHGetPathFromIDListA(pShRet, pThis->m_szIncPath);
				if (fRes) {
					SetDlgItemTextA(hDlg, DKGC_INC_PATH, pThis->m_szIncPath);
				}
				CoTaskMemFree(pShRet);
			}
			break;

		case DKGC_GCC_BROW:
			RtlZeroMemory(pThis->m_szGccPath, sizeof(pThis->m_szGccPath));
			bi.lpszTitle = "Select directory of GCC";
			bi.ulFlags &= ~BIF_NEWDIALOGSTYLE;
			pShRet = SHBrowseForFolderA(&bi);
			if (pShRet) {
				fRes = SHGetPathFromIDListA(pShRet, pThis->m_szGccPath);
				if (fRes) {
					SetDlgItemTextA(hDlg, DKGC_GCC_PATH, pThis->m_szGccPath);
				}
				CoTaskMemFree(pShRet);
			}
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
	return (INT_PTR)FALSE;
}

bool NAppGen::_CheckInput(HWND hDlg)
{
	CHAR szBuf[512];
	UINT uRes = 0;
	
	if (m_bIncSolution) {
		RtlZeroMemory(m_szSlnName, sizeof(m_szSlnName));
		uRes = GetDlgItemTextA(hDlg, DKGC_SLN_NAME, m_szSlnName, sizeof(m_szSlnName));
		if (uRes == 0) {
			MessageBox(hDlg, _T("Solution name can't be empty!"), _T("Error"), MB_OK | MB_ICONERROR);
			return false;
		}
	}

	RtlZeroMemory(m_szProjName, sizeof(m_szProjName));
	uRes = GetDlgItemTextA(hDlg, DKGC_PROJ_NAME, m_szProjName, sizeof(m_szProjName));
	if (uRes == 0) {
		MessageBox(hDlg, _T("Project name can't be empty!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	HRESULT	hr = 0;

	RtlZeroMemory(szBuf, sizeof(szBuf));
	uRes = GetDlgItemTextA(hDlg, DKGC_PROJ_PATH, szBuf, sizeof(szBuf));
	if (uRes == 0) {
		MessageBox(hDlg, _T("Solution and/or Project directory can't be empty!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	} else {
		if (lstrlenA(m_szProjPath) == 0) {
			hr = StringCbCopyA(m_szProjPath, sizeof(m_szProjPath), szBuf);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				MessageBox(hDlg, _T("Can't copy solution and/or project dir. to buffer!"), _T("Error"), MB_OK | MB_ICONERROR);
				return false;
			}
		}
	}

	RtlZeroMemory(szBuf, sizeof(szBuf));
	uRes = GetDlgItemTextA(hDlg, DKGC_INC_PATH, szBuf, sizeof(szBuf));
	if (uRes == 0) {
		MessageBox(hDlg, _T("NanoOS source directory can't be empty!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	} else {
		if (lstrlenA(m_szIncPath) == 0) {
			hr = StringCbCopyA(m_szIncPath, sizeof(m_szIncPath), szBuf);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				MessageBox(hDlg, _T("Can't copy Include dir. to buffer!"), _T("Error"), MB_OK | MB_ICONERROR);
				return false;
			}
		}
	}

	RtlZeroMemory(szBuf, sizeof(szBuf));
	uRes = GetDlgItemTextA(hDlg, DKGC_GCC_PATH, szBuf, sizeof(szBuf));
	if (uRes == 0) {
		MessageBox(hDlg, _T("GCC directory can't be empty!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	} else {
		if (lstrlenA(m_szGccPath) == 0) {
			hr = StringCbPrintfA(m_szGccPath, sizeof(m_szGccPath), szBuf);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				MessageBox(hDlg, _T("Can't copy GCC dir. to buffer!"), _T("Error"), MB_OK | MB_ICONERROR);
				return false;
			}
		}
	}

	return true;
}

bool NAppGen::Generate(HWND hWnd, bool fOverwrite)
{
	bool bRes = false;
	RPC_STATUS rpStat = 0;
	INT_PTR iRes = 0;

	m_pszProjUUID = NULL;

	do {
		rpStat = UuidCreate(&m_ProjUUID);
		if (rpStat != RPC_S_OK) {
			m_dwError = rpStat;
			break;
		}

		rpStat = UuidToStringA(&m_ProjUUID, &m_pszProjUUID);
		if (rpStat != RPC_S_OK) {
			m_dwError = rpStat;
			break;
		}

		iRes = DialogBoxParam(
						GetModuleHandle(NULL), 
						MAKEINTRESOURCE(DKD_GEN_CODE), 
						hWnd, 
						NAppGen::_DlgProc, 
						(LPARAM) this
						);
		if (iRes == DKGC_BTN_CANCEL) {
			m_dwError = ERROR_CANCELLED;
			break;
		}

		if (m_bIncSolution) {
			bRes = _CreateSolution(fOverwrite);
			if (!bRes) break;
		}

		bRes = _CreateProject(fOverwrite);
		if (!bRes) break;

		bRes = _CreateMakefile(fOverwrite);
		if (!bRes) break;

		bRes = _CreateFile(fOverwrite, "Main.c", DKF_MAIN_RES_ID, DKF_MAIN_RES_TYPE);
		if (!bRes) break;

		bRes = _CreateFile(fOverwrite, "Sources.inc", DKF_SRC_INC_RES_ID, DKF_SRC_INC_RES_TYPE);
		if (!bRes) break;

	} while (FALSE);

	if (m_pszProjUUID) {
		RpcStringFreeA(&m_pszProjUUID);
	}

	return bRes;
}

const TCHAR * NAppGen::GetProjectPath()
{
	if (lstrlenA(m_szProjPath) == 0)
		return NULL;

#ifdef UNICODE
	RtlZeroMemory(m_szProjectPath, sizeof(m_szProjectPath));
	int i = MultiByteToWideChar(
				CP_ACP, 
				MB_PRECOMPOSED, 
				m_szProjPath, -1, 
				m_szProjectPath, 
				sizeof(m_szProjectPath)/sizeof(TCHAR)
				);
	if (i > 0)
		return &(m_szProjectPath[0]);
	else
		return NULL;
#else
	return &(m_szProjPath[0]);
#endif
}

DWORD NAppGen::GetError()
{
	return m_dwError;
}