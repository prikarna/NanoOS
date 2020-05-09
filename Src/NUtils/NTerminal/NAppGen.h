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
	LPVOID			m_pMainFileRes;
	DWORD			m_dwMainFileSize;
	LPVOID			m_pSrcIncFile;
	DWORD			m_dwSrcIncSize;
	bool			m_bIncSolution;
	TCHAR			m_szProjectPath[512];

	//bool _GetPathFromFullFileName(const CHAR *szFullFileName, PUCHAR pBuffer, int iBufferLen);
	bool _CreateSolution(bool fOverwrite);
	bool _CreateProject(bool fOverwrite);
	bool _CreateMakefile(bool fOverWrite);
	bool _LoadResource(HMODULE hMod, WORD wResId, WORD wResType, LPVOID * ppRetDat, LPDWORD pdwResSize);
	bool _CreateFile(bool fOverWrite, const char *szFileName, WORD wResId, WORD wResType);
	bool _CheckInput(HWND hDlg);

	static INT_PTR CALLBACK _DlgProc(HWND hDlg, UINT uMsg, WPARAM wParm, LPARAM lParm);

	static const char *	m_szSln[DKS_SLN_MAX_LINES];
	static const char * m_szProj[DKS_PROJ_MAX_LINES];
	static const char * m_szMkfile[DKS_MKF_MAX_LINES];
};
