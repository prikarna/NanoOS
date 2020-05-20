#include "NAppGen.h"
#include "Debug.h"

NAppGen::NAppGen():
m_pCMainFileRes(NULL),
m_dwCMainFileSize(0),
m_pCSrcIncFile(NULL),
m_dwCSrcIncSize(0),
m_pCppMainFileRes(NULL),
m_dwCppMainFileSize(0),
m_pCppSrcIncRes(NULL),
m_dwCppSrcIncSize(0),
m_bIncSolution(false),
m_bIsCPPProj(false)
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

bool NAppGen::_LoadResource(HMODULE hMod, WORD wResId, LPVOID * ppRetRes, LPDWORD pdwResSize)
{
	if ((!ppRetRes) || (!pdwResSize)) {
		m_dwError = ERROR_INVALID_PARAMETER;
		return false;
	}

	HRSRC hRes = FindResource(hMod, MAKEINTRESOURCE(wResId), MAKEINTRESOURCE(DKF_RES_TYPE));
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

	HMODULE	hMod = NULL;
	bool	bRes = false;
	m_dwError = ERROR_SUCCESS;

	do {
		hMod = GetModuleHandle(NULL);
		if (!hMod) {
			m_dwError = GetLastError();
			break;
		}

		bRes = _LoadResource(hMod, DKF_CMAIN_RES_ID, &m_pCMainFileRes, &m_dwCMainFileSize);
		if (!bRes) break;
		
		bRes = _LoadResource(hMod, DKF_CSRC_INC_RES_ID, &m_pCSrcIncFile, &m_dwCSrcIncSize);
		if (!bRes) break;

		bRes = _LoadResource(hMod, DKF_CPPMAIN_RES_ID, &m_pCppMainFileRes, &m_dwCppMainFileSize);
		if (!bRes) break;

		bRes = _LoadResource(hMod, DKF_CPPSRC_INC_RES_ID, &m_pCppSrcIncRes, &m_dwCppSrcIncSize);
	} while (FALSE);

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

		case DKS_PROJ_SRC_FILE:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			if (!m_bIsCPPProj) {
				hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s.\\Main.c\"\n", pszLine, m_szIncPath);
			} else {
				hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s.\\Main.cpp\"\n", pszLine, m_szIncPath);
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

bool NAppGen::_CreateMakefile(bool fOverWrite)
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
	if (fOverWrite) {
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

	for (UINT uiResId = 0; uiResId < DKS_MKF_HDR_MAX_LINES; uiResId++)
	{
		pszLine = const_cast<CHAR *>(m_szMkFlHdr[uiResId]);

		m_dwError = 0;
		switch (uiResId)
		{
		case DKS_MKF_HDR_TGT_NAME:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s%s\n", pszLine, m_szProjName);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				fRes = FALSE;
				break;
			}
			fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			break;

		case DKS_MKF_HDR_OUT_DIR:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s%s\\Bin\n", pszLine, m_szProjPath);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				fRes = FALSE;
				break;
			}
			fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			break;

		case DKS_MKF_HDR_SRCS_DIR:
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

		case DKS_MKF_HDR_INCLUDE:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s%s\\Application\n", pszLine, m_szIncPath);
			if (hr != S_OK) {
				m_dwError = HRESULT_CODE(hr);
				fRes = FALSE;
				break;
			}
			fRes = WriteFile(hFile, (LPCVOID) m_szBuff, lstrlenA(m_szBuff), &dwWr, NULL);
			break;

		case DKS_MKF_HDR_GCC_DIR:
			RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
			hr = StringCbPrintfA(m_szBuff, sizeof(m_szBuff), "%s%s\n", pszLine, m_szGccPath);
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

	if ((fRes == TRUE) && (m_dwError == NO_ERROR))
	{
		dwWr = 0;
		if (m_bIsCPPProj) {
			fRes = WriteFile(hFile, (LPCVOID) m_szMkFlCppBody, lstrlenA(m_szMkFlCppBody), &dwWr, NULL);
		} else {
			fRes = WriteFile(hFile, (LPCVOID) m_szMkFlCBody, lstrlenA(m_szMkFlCBody), &dwWr, NULL);
		}

		if (!fRes) m_dwError = GetLastError();
	}

	CloseHandle(hFile);

	return ((fRes) ? true : false);
}

bool NAppGen::_CreateSourceFile(bool fOverwrite, const char *szFileName, WORD wResId)
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
			break;
		}

		m_dwError = 0;
		switch (wResId)
		{
		case DKF_CMAIN_RES_ID:
			fRes = WriteFile(hFile, reinterpret_cast<LPCVOID>(m_pCMainFileRes), m_dwCMainFileSize, &dwWr, NULL);
			break;

		case DKF_CSRC_INC_RES_ID:
			fRes = WriteFile(hFile, reinterpret_cast<LPCVOID>(m_pCSrcIncFile), m_dwCSrcIncSize, &dwWr, NULL);
			break;

		case DKF_CPPMAIN_RES_ID:
			fRes = WriteFile(hFile, reinterpret_cast<LPCVOID>(m_pCppMainFileRes), m_dwCppMainFileSize, &dwWr, NULL);
			break;

		case DKF_CPPSRC_INC_RES_ID:
			fRes = WriteFile(hFile, reinterpret_cast<LPCVOID>(m_pCppSrcIncRes), m_dwCppSrcIncSize, &dwWr, NULL);
			break;

		default:
			m_dwError = ERROR_INVALID_PARAMETER;
			break;
		}

	} while (FALSE);

	if (!fRes) {
		if (m_dwError != ERROR_INVALID_PARAMETER)
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
		if (pThis->m_bIsCPPProj) {
			hCh = GetDlgItem(hDlg, DKGC_CPP_LANG_RB);
		} else {
			hCh = GetDlgItem(hDlg, DKGC_C_LANG_RB);
		}
		SendMessage(hCh, BM_SETCHECK, static_cast<WPARAM>(BST_CHECKED), 0);
		hCh = GetDlgItem(hDlg, DKGC_SLN_CHECK);
		if (pThis->m_bIncSolution) {
			SendMessage(hCh, BM_SETCHECK, static_cast<WPARAM>(BST_CHECKED), 0);
		} else {
			SendMessage(hCh, BM_SETCHECK, static_cast<WPARAM>(BST_UNCHECKED), 0);
		}

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

		case DKGC_C_LANG_RB:
			DBG_PRINTF(_T("%s: C Lang. selected\r\n"), FUNCT_NAME_STR);
			pThis->m_bIsCPPProj = false;
			break;

		case DKGC_CPP_LANG_RB:
			DBG_PRINTF(_T("%s: C++ Lang. selected\r\n"), FUNCT_NAME_STR);
			pThis->m_bIsCPPProj = true;
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
		iRes = DialogBoxParam(
						GetModuleHandle(NULL), 
						MAKEINTRESOURCE(DKD_GEN_CODE), 
						hWnd, 
						NAppGen::_DlgProc, 
						(LPARAM) this
						);
		if (iRes != DKGC_BTN_OK) {
			if (iRes == DKGC_BTN_CANCEL) {
				m_dwError = ERROR_CANCELLED;
			} else {
				m_dwError = GetLastError();
			}
			break;
		}

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

		if (m_bIncSolution) {
			bRes = _CreateSolution(fOverwrite);
			if (!bRes) break;
		}

		bRes = _CreateProject(fOverwrite);
		if (!bRes) break;

		bRes = _CreateMakefile(fOverwrite);
		if (!bRes) break;

		if (m_bIsCPPProj) {
			bRes = _CreateSourceFile(fOverwrite, "Main.cpp", DKF_CPPMAIN_RES_ID);
			if (!bRes) break;

			bRes = _CreateSourceFile(fOverwrite, "Sources.inc", DKF_CPPSRC_INC_RES_ID);
			if (!bRes) break;
		} else {
			bRes = _CreateSourceFile(fOverwrite, "Main.c", DKF_CMAIN_RES_ID);
			if (!bRes) break;

			bRes = _CreateSourceFile(fOverwrite, "Sources.inc", DKF_CSRC_INC_RES_ID);
			if (!bRes) break;
		}

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