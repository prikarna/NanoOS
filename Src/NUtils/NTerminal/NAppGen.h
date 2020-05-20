#pragma once

#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"
#include "Rpc.h"
#include "ShlObj.h"

#include "ResId.h"

#pragma comment(lib, "Rpcrt4.lib")

class NAppGen
{
public:
	NAppGen();
	~NAppGen();

	bool Generate(HWND hWnd, bool fOverwrite);
	bool Initialize();
	const TCHAR * GetProjectPath();
	DWORD GetError();

private:
	CHAR			m_szSlnName[256];
	CHAR			m_szProjName[256];
	CHAR			m_szProjPath[512];
	CHAR			m_szIncPath[512];
	CHAR			m_szGccPath[512];
	CHAR			m_szBuff[1024];
	DWORD			m_dwError;
	UUID			m_ProjUUID;
	RPC_CSTR		m_pszProjUUID;
	LPVOID			m_pCMainFileRes;
	DWORD			m_dwCMainFileSize;
	LPVOID			m_pCSrcIncFile;
	DWORD			m_dwCSrcIncSize;
	LPVOID			m_pCppMainFileRes;
	DWORD			m_dwCppMainFileSize;
	LPVOID			m_pCppSrcIncRes;
	DWORD			m_dwCppSrcIncSize;
	bool			m_bIncSolution;
	TCHAR			m_szProjectPath[512];
	bool			m_bIsCPPProj;

	bool _CreateSolution(bool fOverwrite);
	bool _CreateProject(bool fOverwrite);
	bool _CreateMakefile(bool fOverWrite);
	bool _LoadResource(HMODULE hMod, WORD wResId, LPVOID * ppRetRes, LPDWORD pdwResSize);
	bool _CreateSourceFile(bool fOverWrite, const char *szFileName, WORD wResId);
	bool _CheckInput(HWND hDlg);

	static INT_PTR CALLBACK _DlgProc(HWND hDlg, UINT uMsg, WPARAM wParm, LPARAM lParm);

	static const char *	m_szSln[DKS_SLN_MAX_LINES];
	static const char * m_szProj[DKS_PROJ_MAX_LINES];
	static const char * m_szMkFlHdr[DKS_MKF_HDR_MAX_LINES];
	static const char m_szMkFlCBody[];
	static const char m_szMkFlCppBody[];
};
