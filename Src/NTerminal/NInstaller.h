/*
 * File    : NInstaller.h
 * Remark  : NInstaller class definition.
 *           To install NanoOS application to NanoOS.
 *
 */

#pragma once
#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"
#include "Dbt.h"

#include "NPort.h"
#include "Debug.h"

class NInstaller
{
private:
	NPort				m_Port;
	TCHAR				m_szAppFileName[1024];
	TCHAR				m_szErrorBuffer[1024];
	bool				m_fReInstall;
	volatile HANDLE		m_hInstThread;
	volatile bool		m_bCancel;
	NPort *				m_pExtPort;
	volatile bool		m_bUseExtPort;
	volatile bool		m_bPrevReadEvent;

	void					_TriggerError(const TCHAR * szErrMsg, DWORD dwError);
	static DWORD CALLBACK	_Install(LPVOID pDat);
	bool					_GetResponse(DWORD *pdwErr);
	bool					_EraseFlash(HANDLE hAppFile, PLARGE_INTEGER pliFileSize, DWORD * pdwErr);
	bool					_ProgramFlash(HANDLE hAppFile, LARGE_INTEGER liFileSize, DWORD * pdwErr);
	static DWORD CALLBACK	_Install2(LPVOID pDat);

	bool				_GetIsInstalling();

public:
	NInstaller(void);
	~NInstaller(void);

/*
	Prop.		: IsInstalling
	Var. type	: bool
	Direction	: get.
	Desc.		: Installation process status, true if still installing or otherwise false.
*/
	Property<NInstaller, bool>	IsInstalling;

/*
	Event			: OnError
	Desc.			: Triggered when error occur.
	Handler param.	: string error message.
*/
	Event<const TCHAR *>		OnError;

/*
	Event			: OnInstalling
	Desc.			: Triggered when install operation is on progress.
	Handler param.	: int variable of progress in percent (%).
*/
	Event<>						OnInstalling;

/*
	Funct.  : bool Install(const TCHAR * szNanoOSIoName, const TCHAR * szAppFileName) 
	Desc.   : 
			Start NanoOS application installation in another thread.
	Params. :
		szNanoOSIoName
			Name of the NanoOS port name.
		szAppFileName
			NanoOS application file name.
	Return  : true if success otherwise false.
*/
	bool Install(const TCHAR * szNanoOSIoName, const TCHAR * szAppFileName);

/*
	Funct.  : Cancel
	Desc.   : Cancel current installation process.
	Params. : None.
	Return  : true if success otherwise false.
*/
	bool Cancel();

/*
	Funct.	: ReInstall
	Desc.   : Start NanoOS application re-installation in another thread.
	Params. :
		szNanoOSIoName
			Name of the NanoOS port name.
	Return  : true if success otherwise false.
*/
	bool ReInstall(const TCHAR * szNanoOSIoName);

/*
	Funct.	: Install
	Desc.   : Installation operation thread function (overloaded).
	Params. :
		pPort
			Pointer to existing instance of NPort object.
		szAppFileName
			String NanoOS application file.
	Return  : true if success otherwise false.
*/
	bool Install(NPort * pPort, const TCHAR * szAppFileName);

/*
	Funct.	: ReInstall
	Desc.   : Re-installation operation (overloaded).
	Params. : None.
	Return  : true if success otherwise false.
*/
	bool ReInstall();
};
