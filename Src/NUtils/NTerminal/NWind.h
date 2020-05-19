/*
 * File    : NWind.h
 * Remark  : NWind class definition.
 *           Provide GUI for NanoOS Terminal application with MDI style.
 *
 */

#pragma once

#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"
#include "Dbt.h"
#include "WinIoCtl.h"

#include "VDisplay.h"
#include "NTerminal.h"
#include "NInstaller.h"
#include "NPort.h"
#include "NAppGen.h"

#include "..\NPortSvc\NSvcControl.h"

#include "..\..\UsbData.h"
#include "..\..\Application\VDispData.h"

class NWind:
	public EventConsumer
{
public:
	enum UsbIoMode {
		UsbIoMod_None = 0,
		UsbIoMod_SerialIo,
		UsbIoMod_VDisplayIo
	};

private:
	HINSTANCE			m_hInst;
	HWND				m_hWnd;
	HWND				m_hClientWnd;
	CLIENTCREATESTRUCT	m_ccs;
	OPENFILENAME		m_OFN;
	TCHAR				m_szAppFullFileName[512];
	TCHAR				m_szAppFileName[512];
	TCHAR				m_szSelCOM[512];
	TCHAR				m_szBuff[1024];
	const TCHAR	*		m_szClassName;
	int					m_iHeight;
	int					m_iWidth;
	HMENU				m_hMenu;
	HMENU				m_hConsMenu;
	bool				m_fNeedUpdateMenu;
	int					m_iConsMenuId;
	HDEVNOTIFY			m_hConsNotif;

	const TCHAR *		m_szNanoOSPortName;

	NPort				m_ConsPort;
	NPort				m_NanoOSPort;

	NTerminal			m_Console;
	NTerminal			m_UsbSerIo;

	NInstaller			m_Installer;

	USB_DATA			m_UsbDat;

	UsbIoMode			m_CurIoMode;
	UsbIoMode			m_PrevIoMode;

	VDisplay			m_VDisp;
	NSvcControl			m_SvcCtl;
	NAppGen				m_AppGen;

	static LRESULT CALLBACK _WindProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm);
	static INT_PTR CALLBACK _AboutDlgProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm);

	void _UpdateConsoleMenu();
	void _HandleCommand(WPARAM wParm, LPARAM lParm);
	void _HandleCOMxMenu(UINT uMenuId);
	bool _GetFileNameFromFullPath(const TCHAR *szFullPathFileName, TCHAR * szBuffer, int iByteBufferSize);
	bool _GetFilePathFromFullPath(const TCHAR *szFullPathFileName, TCHAR * szBuffer, int iByteBufferSize);
	void _ReOpenConsolePort();

	void _HandleOnConsPortEnum(const TCHAR *szCOMName);
	void _HandleOnConsPortDataReceived(char * pDat);
	void _HandleOnConsPortError(const TCHAR * szErrMsg);
	void _HandleOnConsChar(CHAR ch);

	void _HandleOnUsbSerIoChar(CHAR ch);

	void _HandleOnNanoOSPortDataReceived(char * pDat);
	void _HandleOnNanoOSPortDeviceChange(bool isConected);

	void _HandleOnProgressInstall(int iProgress);
	void _HandleOnInstallError(const TCHAR * szErrMsg);

	void _HandleOnVDisplayInput(VDisplay::InputData * pDat);

	void _ArrangeChilds();
	void _ChangeUsbIoMode(NWind::UsbIoMode mode, bool bCompareCurMode);

	bool _InstallService();
	bool _UninstallService();

	bool _StartService();
	bool _SuspendService();
	bool _ResumeService();
	bool _StopService();

	bool _GetServiceStatus();

	void _HandleOnSvcControlError(const TCHAR * szErr);

public:
	NWind();	// C'tor
	~NWind();	// D'tor

/*
	Funct.	: Create
	Desc.   : Create frame window and its components.
	Params. :
		hInst
			Handle to instance of current process.
	Return  : true if success otherwise false.
*/
	bool Create(HINSTANCE hInst);

/*
	Funct.	: Show
	Desc.   : Show or display created window.
	Params. : None.
	Return  : Message status.
*/
	int Show();
};
