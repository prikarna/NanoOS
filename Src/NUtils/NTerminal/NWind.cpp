/*
 * File    : NWind.cpp
 * Remark  : NWind class implementation.
 *           Provide GUI for NanoOS Terminal application with MDI style.
 *
 */

#include "NWind.h"
#include "ResId.h"
#include "Debug.h"
#include "NTermVer.h"
#include "NPortName.h"

/* C'tor */
NWind::NWind():
	m_hInst(NULL),
	m_hWnd(NULL),
	m_hClientWnd(NULL),
	m_szClassName(_T("NWindClass")),
	m_iHeight(480),
	m_iWidth(800),
	m_hMenu(NULL),
	m_fNeedUpdateMenu(false),
	m_hConsNotif(NULL),
	m_CurIoMode(NWind::UsbIoMod_None),
	m_PrevIoMode(NWind::UsbIoMod_None)
{
	m_szNanoOSPortName = NPORT_DEVICE_NAME;

	RtlZeroMemory(&m_OFN, sizeof(OPENFILENAME));
	RtlZeroMemory(m_szAppFullFileName, sizeof(m_szAppFullFileName));
	RtlZeroMemory(m_szAppFileName, sizeof(m_szAppFileName));
	RtlZeroMemory(m_szSelCOM, sizeof(m_szSelCOM));
	RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
}

/* D'tor */
NWind::~NWind()
{
}

bool NWind::Create(HINSTANCE hInst)
{
	WNDCLASSEX			wcx = {0};
	ATOM				at = 0;
	LRESULT				lr = 0;
	DWORD				dwErr = 0;
	RECT				rc = {0};
	HANDLE				hImg = NULL;

	if (m_hWnd) return false;

	hImg = LoadImage(hInst, MAKEINTRESOURCE(DKICON_APP), IMAGE_ICON, 48, 48, (LR_VGACOLOR));

	wcx.cbSize			= sizeof(WNDCLASSEX);
	wcx.hbrBackground	= reinterpret_cast<HBRUSH>(COLOR_WINDOW);
	wcx.hCursor			= LoadCursor(NULL, IDC_ARROW);
	if (hImg) {
		wcx.hIcon		= static_cast<HICON>(hImg);
	} else {
		wcx.hIcon		= LoadIcon(NULL, IDI_APPLICATION);
	}
	wcx.hInstance		= hInst;
	wcx.lpfnWndProc		= reinterpret_cast<WNDPROC>(&NWind::_WindProc);
	wcx.lpszClassName	= m_szClassName;
	wcx.lpszMenuName	= _T("MainMenu");
	wcx.style			= CS_HREDRAW | CS_VREDRAW;

	at = RegisterClassEx(&wcx);
	if (at <= 0){
		MessageBox(NULL, _T("Error register window class!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	int	iScX = GetSystemMetrics(SM_CXFULLSCREEN);
	int iScY = GetSystemMetrics(SM_CYFULLSCREEN);
	int iXPos = (iScX - m_iWidth) / 2;
	int iYPos = (iScY - m_iHeight) / 2;

	SetRect(&rc, iXPos, iYPos, (m_iWidth + iXPos), (m_iHeight + iYPos));
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, FALSE, 0);

	m_hWnd = CreateWindowEx(
						0,
						m_szClassName,
						_T("NanoOS Terminal"),
						WS_OVERLAPPEDWINDOW,
						rc.left,
						rc.top,
						(rc.right - rc.left),
						(rc.bottom - rc.top),
						NULL,
						NULL,
						hInst,
						reinterpret_cast<LPVOID>(this)
						);
	if (!m_hWnd) {
		MessageBox(NULL, _T("Error create window!"), _T("Error"), MB_OK | MB_ICONERROR);
		UnregisterClass(m_szClassName, hInst);
		return false;
	}

	m_hMenu = GetMenu(m_hWnd);

	m_ccs.hWindowMenu = GetSubMenu(m_hMenu, 1);
	m_ccs.idFirstChild = FIRST_WND_CHILD_ID;
	m_hClientWnd =
		CreateWindowEx(
					WS_EX_CLIENTEDGE,
					_T("MDICLIENT"),
					NULL,
					WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE,
					0, 0, 0, 0,
					m_hWnd,
					NULL,
					hInst,
					reinterpret_cast<LPVOID>(&m_ccs)
					);
	if (!m_hClientWnd) {
		MessageBox(NULL, _T("Error create client window!"), _T("Error"), MB_OK | MB_ICONERROR);
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
		UnregisterClass(m_szClassName, hInst);

		return false;
	}

	m_hInst = hInst;

	m_OFN.hwndOwner		= m_hWnd;
	m_OFN.Flags			= OFN_FILEMUSTEXIST | 
						  OFN_NONETWORKBUTTON | 
						  OFN_DONTADDTORECENT | 
						  OFN_READONLY | 
						  OFN_PATHMUSTEXIST;
	m_OFN.hInstance		= hInst;
	m_OFN.lpstrFile		= &m_szAppFullFileName[0];
	m_OFN.lStructSize	= sizeof(OPENFILENAME);
	m_OFN.lpstrTitle	= _T("Select NanoOS Application File");
	m_OFN.nMaxFile		= sizeof(m_szAppFullFileName)/sizeof(TCHAR);
	m_OFN.lpstrFilter	= _T("Binary File\0*.bin\0\0");
	
	HMENU hSubMenu = GetSubMenu(m_hMenu, 0);
	m_hConsMenu = GetSubMenu(hSubMenu, 0);

	m_Console.Create(hInst, m_hClientWnd, _T("Console"));
	m_Console.OnChar = new EventHandler<CHAR>(this, &NWind::_HandleOnConsChar);
	m_ConsPort.OnEnumCOM = new EventHandler<const TCHAR *>(this, &NWind::_HandleOnConsPortEnum);
	m_ConsPort.OnDataReceived = new EventHandler<char *>(this, &NWind::_HandleOnConsPortDataReceived);
	m_ConsPort.OnError = new EventHandler<const TCHAR *>(this, &NWind::_HandleOnConsPortError);
	
	m_fNeedUpdateMenu = true;
	_UpdateConsoleMenu();

	m_Installer.OnError = new EventHandler<const TCHAR *>(this, &NWind::_HandleOnInstallError);
	m_Installer.OnInstalling = new EventHandler<>(this, &NWind::_HandleOnProgressInstall);

	m_UsbSerIo.OnChar = new EventHandler<CHAR>(this, &NWind::_HandleOnUsbSerIoChar);
	
	m_NanoOSPort.OnDataReceived = 
		new EventHandler<char *>(this, &NWind::_HandleOnNanoOSPortDataReceived);
	m_NanoOSPort.OnDeviceChange = 
		new EventHandler<bool>(this, &NWind::_HandleOnNanoOSPortDeviceChange);
	m_NanoOSPort.ReadDataSize = (USB_DATA_SIZE - 2);
	m_NanoOSPort.TimeOut = 1;

	m_UsbSerIo.Create(hInst, m_hClientWnd, _T("USB Serial IO [Inactive]"));
	m_VDisp.Create(hInst, m_hClientWnd, _T("USB VDisplay IO [Inactive]"), 40, 40, 240, 320);
	m_VDisp.FillRectangle(NULL, RGB(255,255,255));

	bool	bRes = false, bIsInstalled = false;
	NSvcControl::SvcStatus	stat = NSvcControl::Stopped;
	bRes = m_SvcCtl.IsInstalled(bIsInstalled);
	if (bRes) {
		if (bIsInstalled) {
			bRes = m_SvcCtl.GetStatus(stat);
		}
	}
	if (bRes && (stat == NSvcControl::Running)) {
		m_PrevIoMode = NWind::UsbIoMod_SerialIo;
		_ChangeUsbIoMode(NWind::UsbIoMod_None, false);
	} else {
		_ChangeUsbIoMode(NWind::UsbIoMod_SerialIo, false);
	}

	m_VDisp.OnInput = new EventHandler<VDisplay::InputData *>(this, &NWind::_HandleOnVDisplayInput);

	m_AppGen.Initialize();

	//m_SvcCtl.OnError = new EventHandler<const TCHAR *>(this, &NWind::_HandleOnSvcControlError);

	return true;
}

int NWind::Show()
{
	MSG		sMsg;

	if (!m_hWnd) return -1;

	HACCEL	hAccel = LoadAccelerators(m_hInst, _T("MenuAccel"));
	if (!hAccel) 
		MessageBox(m_hWnd, _T("Warning: Fail to load accelerator!"), _T("Warning"), MB_OK | MB_ICONWARNING);

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	_ArrangeChilds();

	while (GetMessage(&sMsg, (HWND) NULL, 0, 0) > 0)
	{
		if (!TranslateMDISysAccel(m_hClientWnd, &sMsg) &&
			!TranslateAccelerator(m_hWnd, hAccel, &sMsg))
		{ 
			TranslateMessage(&sMsg); 
			DispatchMessage(&sMsg); 
		} 
	}

	UnregisterClass(m_szClassName, m_hInst);
	m_hWnd = NULL;

	return(static_cast<int>(sMsg.wParam));
}

LRESULT CALLBACK NWind::_WindProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm)
{
	static NWind *		pThis = NULL;
	LPCREATESTRUCT		pcs = NULL;
	int					iRes;

	switch (uMsg) 
	{
	case WM_CREATE:
		pcs = reinterpret_cast<LPCREATESTRUCT>(lParm);
		if (pcs) {
			pThis = reinterpret_cast<NWind *>(pcs->lpCreateParams);
		}
		break;

	case WM_CLOSE:
		iRes = MessageBox(
						hWnd, 
						_T("Do you want to quit NanoOS Terminal ?"), 
						_T("Confirmation"), 
						MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION
						);
		if (iRes == IDYES) {
			if (pThis->m_ConsPort.IsOpen)
				pThis->m_ConsPort.Close();

			if (pThis->m_hConsNotif) {
				UnregisterDeviceNotification(pThis->m_hConsNotif);
				pThis->m_hConsNotif = NULL;
			}

			pThis->m_NanoOSPort.DisableAutoDetection();

			DestroyWindow(hWnd);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_INITMENUPOPUP:
		if ((HMENU) wParm == pThis->m_hConsMenu) {
			if (pThis->m_fNeedUpdateMenu) {
				pThis->_UpdateConsoleMenu();
			}
		}
		break;

	case WM_DEVICECHANGE:
		switch (wParm)
		{
		case DBT_DEVICEREMOVECOMPLETE:
			DBG_PRINTF(_T("WM_DEVICECHANGE: Device remove complete.\r\n"));
			if (pThis->m_ConsPort.IsOpen) {
				pThis->m_ConsPort.Close();
				if (pThis->m_hConsNotif) {
					UnregisterDeviceNotification(pThis->m_hConsNotif);
					pThis->m_hConsNotif = NULL;
				}
				pThis->m_fNeedUpdateMenu = true;
				pThis->m_Console.Title = _T("Console");
			}
			break;

		default:
			DBG_PRINTF(_T("WM_DEVICECHANGE: wParm=0x%X\r\n"), wParm);
			pThis->_ReOpenConsolePort();
			break;
		}
		break;

	case WM_COMMAND:
		if (pThis)
			pThis->_HandleCommand(wParm, lParm);
		break;

	default:
		if (pThis) {
			return DefFrameProc(hWnd, pThis->m_hClientWnd, uMsg, wParm, lParm);
		} else {
			return DefFrameProc(hWnd, NULL, uMsg, wParm, lParm);
		}
	}

	if ((uMsg == WM_COMMAND) ||
		(uMsg == WM_MENUCHAR) ||
		(uMsg == WM_SETFOCUS) ||
		(uMsg == WM_SIZE))
	{
		if (pThis) {
			return DefFrameProc(hWnd, pThis->m_hClientWnd, uMsg, wParm, lParm);
		} else {
			return DefFrameProc(hWnd, NULL, uMsg, wParm, lParm);
		}
	}

	return (static_cast<LRESULT>(FALSE));
}

void NWind::_HandleOnConsPortEnum(const TCHAR *szCOMName)
{
	if (!m_hConsMenu) return;

	BOOL	fRes = FALSE;
	fRes = AppendMenu(m_hConsMenu, MF_STRING, m_iConsMenuId, szCOMName);
	if (fRes) {
		if (lstrcmp(m_szSelCOM, szCOMName) == 0) {
			if (m_ConsPort.IsOpen) {
				MENUITEMINFO			mii;
				RtlZeroMemory(&mii, sizeof(MENUITEMINFO));
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_STATE;
				mii.fState = MFS_CHECKED;
				SetMenuItemInfo(m_hConsMenu, m_iConsMenuId, FALSE, &mii);
			}
		}
		m_iConsMenuId++;
	}
}

void NWind::_UpdateConsoleMenu()
{
	if (!m_fNeedUpdateMenu) return;

	if (!m_hConsMenu) return;

	BOOL	fRes = TRUE;
	while (fRes) {
		fRes = DeleteMenu(m_hConsMenu, 0, MF_BYPOSITION);
	}

	m_iConsMenuId = DKM_APP_FIRST_COM;
	m_ConsPort.EnumComm();

	if (m_iConsMenuId == DKM_APP_FIRST_COM) {
		fRes = AppendMenu(m_hConsMenu, MF_SEPARATOR, 0, NULL);
	}

	m_fNeedUpdateMenu = false;
}

void NWind::_HandleCommand(WPARAM wParm, LPARAM lParm)
{
	BOOL		fRes;
	HWND		hWnd;
	int			iRes = 0;

	if ((LOWORD(wParm) >= DKM_APP_FIRST_COM) &&
		(LOWORD(wParm) < m_iConsMenuId))
	{
		_HandleCOMxMenu(LOWORD(wParm));
		return;
	}

	switch (LOWORD(wParm))
	{
	case DKM_APP_INSTALL:
		RtlZeroMemory(m_szAppFullFileName, sizeof(m_szAppFullFileName));
		fRes = GetOpenFileName(&m_OFN);
		if (fRes) {
			_GetFileNameFromFullPath(m_szAppFullFileName, m_szAppFileName, sizeof(m_szAppFileName));
			
			if (m_Installer.IsInstalling) {
				iRes = MessageBox(
								m_hWnd, 
								_T("Still installing. Do you want to cancel current install operation ?"), 
								_T("Confirmation"),
								MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2
								);
				if (iRes == IDNO) {
					SetWindowText(m_hWnd, _T("NanoOS Terminal"));
					break;
				}

				m_Installer.Cancel();
			}

			m_NanoOSPort.Purge(NPort::PurgeOption_All);

			SetWindowText(m_hWnd, _T("NanoOS Terminal - Installing"));

			m_Console.Printf(_T("Install %s to NanoOS... "), m_szAppFileName);
			m_Installer.Install(&m_NanoOSPort, m_szAppFullFileName);

			hWnd = m_Console.Handle;
			SendMessage(m_hClientWnd, WM_MDIACTIVATE, reinterpret_cast<WPARAM>(hWnd), 0);
		}
		break;


	case DKM_APP_REINSTALL:
		if (lstrlen(m_szAppFileName) > 0) {

			if (m_Installer.IsInstalling) {
				iRes = MessageBox(
								m_hWnd, 
								_T("Still installing. Do you want to cancel current install operation ?"), 
								_T("Confirmation"),
								MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2
								);
				if (iRes == IDNO) {
					SetWindowText(m_hWnd, _T("NanoOS Terminal"));
					break;
				}

				m_Installer.Cancel();
			}

			SetWindowText(m_hWnd, _T("NanoOS Terminal - Installing"));

			m_NanoOSPort.Purge(NPort::PurgeOption_All);

			m_Console.Printf(_T("Re-Install %s to NanoOS... "), m_szAppFileName);
			m_Installer.ReInstall();

			hWnd = m_Console.Handle;
			SendMessage(m_hClientWnd, WM_MDIACTIVATE, reinterpret_cast<WPARAM>(hWnd), 0);

		} else {
			m_Console.Printf(_T("No previously installed application!\r\n"));
		}
		break;

	case DKM_APP_CANCEL_INSTALL:
		m_Installer.Cancel();
		SetWindowText(m_hWnd, _T("NanoOS Terminal"));
		break;

	case DKM_APP_UPDATE_CONS_MENU:
		m_fNeedUpdateMenu = true;
		break;

	case DKM_GEN_CODE:
		if (m_AppGen.Generate(m_hWnd, true)) {
			m_Console.Printf(_T("Successfully generate NanoOS Application Solution/Project in %s.\r\n"), m_AppGen.GetProjectPath());
		} else {
			if (m_AppGen.GetError() == ERROR_CANCELLED) {
				m_Console.AppendText(_T("Operation cancelled!\r\n"));
			} else {
				m_Console.Printf(_T("Generate code failed! (%d)\r\n"), m_AppGen.GetError());
			}
		}
		break;

	case DKM_APP_EXIT:
		SendMessage(m_hWnd, WM_CLOSE, 0, 0);
		break;

	case DKM_VIEW_DEFAULT_ARRANGEMENT:
		_ArrangeChilds();
		break;

	case DKM_VIEW_CLEAR_CONSOLE:
		m_Console.ClearText();
		break;

	case DKM_VIEW_CLEAR_USB_SERIAL_IO:
		m_UsbSerIo.ClearText();
		break;

	case DKM_VIEW_CLEAR_ALL:
		m_Console.ClearText();
		m_UsbSerIo.ClearText();
		break;

	case DKM_OPT_NPORT_DISABLE:
		_ChangeUsbIoMode(NWind::UsbIoMod_None, true);
		break;

	case DKM_OPT_NPORT_SEL_SERIAL:
		_ChangeUsbIoMode(NWind::UsbIoMod_SerialIo, true);
		break;

	case DKM_OPT_NPORT_SEL_VDISPLAY:
		_ChangeUsbIoMode(NWind::UsbIoMod_VDisplayIo, true);
		break;

	case DKM_OPT_NSVC_STATUS:
		_GetServiceStatus();
		break;

	case DKM_OPT_NSVC_INSTALL:
		_InstallService();
		break;

	case DKM_OPT_NSVC_UNINSTALL:
		_UninstallService();
		break;

	case DKM_OPT_NSVC_START:
		_StartService();
		break;

	case DKM_OPT_NSVC_SUSPEND:
		_SuspendService();
		break;

	case DKM_OPT_NSVC_RESUME:
		_ResumeService();
		break;

	case DKM_OPT_NSVC_STOP:
		_StopService();
		break;

	case DKM_HELP_ABOUT:
		DialogBoxParam(m_hInst, MAKEINTRESOURCE(DKD_ABOUTBOX), m_hWnd, _AboutDlgProc, 0);
		break;

	default:
		break;
	}
}

void NWind::_HandleCOMxMenu(UINT uMenuId)
{
	MENUITEMINFO			mii = {0};
	TCHAR					szMenuName[64] = {0};
	BOOL					fRes = FALSE;
	DEV_BROADCAST_HANDLE	DevBCast = {0};
	bool					bRes;

	if (!m_hConsMenu) return;

	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STRING | MIIM_STATE;
	mii.wID = uMenuId;
	mii.cch = sizeof(szMenuName)/sizeof(TCHAR);
	mii.dwTypeData = szMenuName;
	fRes = GetMenuItemInfo(m_hConsMenu, uMenuId, FALSE, &mii);
	if (!fRes) return;

	if (mii.fState & MFS_CHECKED)
	{
		if (m_ConsPort.IsOpen) {
			if (m_ConsPort.Close()) {

				if (m_hConsNotif) {
					fRes = UnregisterDeviceNotification(m_hConsNotif);
					m_hConsNotif = NULL;
				}

				RtlZeroMemory(&m_szSelCOM, sizeof(m_szSelCOM));
				
				mii.fMask = MIIM_STATE;
				mii.fState &= ~(MFS_CHECKED);
				fRes = SetMenuItemInfo(m_hConsMenu, uMenuId, FALSE, &mii);
				if (fRes) {
					RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
					StringCbPrintf(m_szBuff, sizeof(m_szBuff), _T("Console"));
					m_Console.Title = m_szBuff;
				}
			}
		}
	}
	else
	{
		if (!m_ConsPort.IsOpen) {
			bRes = m_ConsPort.Open(
								szMenuName,
								921600,
								NPort::ByteSize_8,
								NPort::Parity_None,
								NPort::StopBits_1,
								NPort::DTRControl_Enable,
								NPort::RTSControl_Enable
								);
			if (bRes) {

				DevBCast.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
				DevBCast.dbch_devicetype = DBT_DEVTYP_HANDLE;
				DevBCast.dbch_handle = m_ConsPort.Handle;
				m_hConsNotif = RegisterDeviceNotification(m_hWnd, (LPVOID) &DevBCast, DEVICE_NOTIFY_WINDOW_HANDLE);

				mii.fMask = MIIM_STATE;
				mii.fState |= MFS_CHECKED;
				fRes = SetMenuItemInfo(m_hConsMenu, uMenuId, FALSE, &mii);
				if (fRes) {
					RtlZeroMemory(&m_szSelCOM, sizeof(m_szSelCOM));
					StringCbCopy(m_szSelCOM, sizeof(m_szSelCOM), szMenuName);
					
					RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
					StringCbPrintf(m_szBuff, sizeof(m_szBuff), _T("Console on %s"), m_szSelCOM);
					m_Console.Title = m_szBuff;
				}
			}
		}
	}
}

bool NWind::_GetFileNameFromFullPath(const TCHAR *szFullPathFileName, TCHAR *szBuffer, int iByteBufferSize)
{
	int		iFullPathLen = 0;

	if ((!szFullPathFileName) || (!szBuffer)) 
		return false;

	if (iByteBufferSize == 0) return false;

	iFullPathLen = lstrlen(szFullPathFileName);
	if (iFullPathLen == 0) return false;

	int	iFileNameLen = 0;
	TCHAR * pCh = (TCHAR *) &(szFullPathFileName[iFullPathLen - 1]);
	while (*pCh != _T('\\')) 
	{ 
		pCh--;
		iFileNameLen++;
	}
	pCh++;

	if ( static_cast<int>(iFileNameLen * sizeof(TCHAR)) >= iByteBufferSize)
		return false;

	RtlZeroMemory(szBuffer, iByteBufferSize);
	StringCbCopy(szBuffer, iByteBufferSize, pCh);

	return true;
}

bool NWind::_GetFilePathFromFullPath(const TCHAR *szFullPathFileName, TCHAR *szBuffer, int iBufferByteSize)
{
	bool bRes = false;

	if ((szBuffer == NULL) || (iBufferByteSize <= 0)) return false;

	int	iFullPathSize = lstrlen(szFullPathFileName) * sizeof(TCHAR);
	if (iFullPathSize <= 0) return false;

	if (iFullPathSize > iBufferByteSize) return false;

	RtlZeroMemory(szBuffer, iBufferByteSize);
	HRESULT hr = StringCbCopy(szBuffer, iBufferByteSize, szFullPathFileName);
	if (hr != S_OK) return false;

	TCHAR *pChar = &(szBuffer[iFullPathSize - 1]);
	while (*pChar != _T('\\')) {
		*pChar = 0;
		pChar--;
	}
	*pChar = 0;

	return true;
}

void NWind::_HandleOnConsPortDataReceived(char * pData)
{
	m_Console.AppendMultiByteText(pData);
}

void NWind::_HandleOnConsPortError(const TCHAR *szErrMsg)
{
	m_Console.AppendText(_T("Error: "));
	m_Console.AppendText(szErrMsg);
	m_Console.AppendText(_T("\r\n"));
}

void NWind::_HandleOnNanoOSPortDataReceived(char *pDat)
{
	PVDISP_OUT_DATA		pDisp = NULL;
	bool				b = false;
	COLORREF			cr;
	VDISP_IN_DATA		InDat;

	if (m_CurIoMode == NWind::UsbIoMod_SerialIo) 
	{
		m_UsbSerIo.AppendMultiByteText(pDat);
	} 
	else if (m_CurIoMode == NWind::UsbIoMod_VDisplayIo) 
	{
		pDisp = reinterpret_cast<PVDISP_OUT_DATA>(pDat);
		switch (pDisp->Type)
		{
		case VDISP_OUT_TYPE__NONE:
			DBG_PRINTF(_T("%s: Got NONE data out.\r\n"), FUNCT_NAME_STR);
			break;

		case VDISP_OUT_TYPE__SET_PIXEL:
			b = m_VDisp.SetPixel(pDisp->u.Pixel.X, pDisp->u.Pixel.Y, pDisp->u.Pixel.Color);
			break;

		case VDISP_OUT_TYPE__GET_PIXEL:
			b = m_VDisp.GetPixel(pDisp->u.Pixel.X, pDisp->u.Pixel.Y, cr);
			if (b) {
				InDat.Type = VDISP_IN_TYPE__PIXEL;
				InDat.X = pDisp->u.Pixel.X;
				InDat.Y = pDisp->u.Pixel.Y;
				InDat.Color = cr;
				b = m_NanoOSPort.Write(reinterpret_cast<unsigned char *>(&InDat), sizeof(VDISP_IN_DATA));
			}
			break;

		case VDISP_OUT_TYPE__FILL_RECT:
			m_VDisp.FillRectangle((LPRECT) &(pDisp->u.FillRect.Rectangle), pDisp->u.FillRect.Color);
			break;

		case VDISP_OUT_TYPE__UPDATE:
			DBG_PRINTF(_T("%s: Got UPDATE data out.\r\n"), FUNCT_NAME_STR);
			break;

		default:
			break;
		}
	}
	else
	{
		// Do nothing
	}
}

void NWind::_HandleOnConsChar(CHAR ch)
{
	m_ConsPort.Write(ch);
}

void NWind::_HandleOnUsbSerIoChar(CHAR ch)
{
	m_NanoOSPort.Write(ch);
}

void NWind::_HandleOnNanoOSPortDeviceChange(bool isConected)
{
	RECT	rc;
	if (isConected) 
	{
		if (m_CurIoMode == NWind::UsbIoMod_SerialIo) 
		{
			m_UsbSerIo.Title = _T("USB Serial IO - Connected");
		} 
		else if (m_CurIoMode == NWind::UsbIoMod_VDisplayIo) 
		{
			m_VDisp.Title = _T("USB VDisplay - Connected");
			SetRect(&rc, 0, 0, VDISP_WIDTH, VDISP_HEIGHT);
			m_VDisp.FillRectangle(&rc, RGB(0, 0, 0));
		}
		else
		{
			// Do nothing
		}
	} 
	else 
	{
		if (m_CurIoMode == NWind::UsbIoMod_SerialIo) 
		{
			m_UsbSerIo.Title = _T("USB Serial IO - Disconnected");
		} 
		else if (m_CurIoMode == NWind::UsbIoMod_VDisplayIo) 
		{
			m_VDisp.Title = _T("USB VDisplay - Disconnected");
			SetRect(&rc, 0, 0, VDISP_WIDTH, VDISP_HEIGHT);
			m_VDisp.FillRectangle(&rc, RGB(255, 255, 255));
		}
		else
		{
			// Do nothing
		}
	}
}

void NWind::_ReOpenConsolePort()
{
	bool					bRes = false;

	if (lstrlen(m_szSelCOM) <= 0) return;
	if (m_ConsPort.IsOpen) return;

	m_ConsPort.SuppressError = true;

	bRes = m_ConsPort.Open(
						m_szSelCOM,
						921600,
						NPort::ByteSize_8,
						NPort::Parity_None,
						NPort::StopBits_1,
						NPort::DTRControl_Enable,
						NPort::RTSControl_Enable
						);
	if (bRes) {

		DEV_BROADCAST_HANDLE	DevBCast = {0};

		DevBCast.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
		DevBCast.dbch_devicetype = DBT_DEVTYP_HANDLE;
		DevBCast.dbch_handle = m_ConsPort.Handle;
		m_hConsNotif = RegisterDeviceNotification(m_hWnd, (LPVOID) &DevBCast, DEVICE_NOTIFY_WINDOW_HANDLE);

		RtlZeroMemory(m_szBuff, sizeof(m_szBuff));
		StringCbPrintf(m_szBuff, sizeof(m_szBuff), _T("Console on %s"), m_szSelCOM);
		m_Console.Title = m_szBuff;

		m_fNeedUpdateMenu = false;
	}

	m_ConsPort.SuppressError = false;
}

void NWind::_HandleOnProgressInstall(int iProgress)
{
	if (iProgress == 100) {
		m_Console.AppendText(_T(" Success.\r\n"));
		SetWindowText(m_hWnd, _T("NanoOS Terminal"));
	} else {
		m_Console.AppendText(_T("#"));
	}
}

void NWind::_ArrangeChilds()
{
	RECT	rc;
	RECT	rcDisp;
	RECT	rcTerm, rcClTerm;

	int		iTermWidth = 0, iTermHeight = 0, iTermDeltaX = 0, iTermDeltaY;

	if (!m_hWnd) return;

	GetClientRect(m_hWnd, &rc);
	GetWindowRect(m_VDisp.Handle, &rcDisp);
	GetWindowRect(m_Console.Handle, &rcTerm);
	GetClientRect(m_Console.Handle, &rcClTerm);

	iTermDeltaY = (rcTerm.bottom - rcTerm.top) - (rcClTerm.bottom - rcClTerm.top);
	iTermDeltaX = (rcTerm.right - rcTerm.left) - (rcClTerm.right - rcClTerm.left);
	iTermWidth = (rc.right - rc.left) - (rcDisp.right - rcDisp.left);
	iTermWidth -= (iTermDeltaX / 2);
	iTermHeight = (rc.bottom - rc.top) / 2;
	iTermHeight -= (iTermDeltaY / 2);

	MoveWindow(m_Console.Handle, 0, 0, iTermWidth, iTermHeight, TRUE);
	MoveWindow(m_UsbSerIo.Handle, 0, iTermHeight, iTermWidth, iTermHeight, TRUE);
	MoveWindow(m_VDisp.Handle, iTermWidth, 0, (rcDisp.right - rcDisp.left), (rcDisp.bottom - rcDisp.top), TRUE);
}

void NWind::_HandleOnVDisplayInput(VDisplay::InputData *pDat)
{
	VDISP_IN_DATA	Input = {0};
	bool			bRes = false;

	switch (pDat->Msg)
	{
	case WM_LBUTTONDOWN:
		Input.Type = VDISP_IN_TYPE__TOUCH;
		Input.X = LOWORD(pDat->LParam);
		Input.Y = HIWORD(pDat->LParam);
		break;

	case WM_LBUTTONUP:
		Input.Type = VDISP_IN_TYPE__RELEASED;
		Input.X = LOWORD(pDat->LParam);
		Input.Y = HIWORD(pDat->LParam);
		break;

	default:
		Input.Type = VDISP_IN_TYPE__NONE;
		Input.X = LOWORD(pDat->LParam);
		Input.Y = HIWORD(pDat->LParam);
		break;
	}

	bRes = m_NanoOSPort.Write(reinterpret_cast<unsigned char *>(&Input), sizeof(VDISP_IN_DATA));
}

void NWind::_HandleOnInstallError(const TCHAR *szErrMsg)
{
	m_Console.AppendText(_T("Error: "));
	m_Console.AppendText(szErrMsg);
	m_Console.AppendText(_T("\r\n"));

	SetWindowText(m_hWnd, _T("NanoOS Terminal"));
}

INT_PTR CALLBACK NWind::_AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParm, LPARAM lParm)
{
	HWND	hStat;
	TCHAR	szBuf[256];

	switch (uMsg)
	{
	case WM_INITDIALOG:
		RtlZeroMemory(szBuf, sizeof(szBuf));
		StringCbPrintf(szBuf, sizeof(szBuf), _T("NTerminal, Version %s"), NTERMVER_STR);
		hStat = GetDlgItem(hDlg, DKC_STATIC_2);
		if (hStat) {
			SetWindowText(hStat, szBuf);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParm) == IDOK || LOWORD(wParm) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParm));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void NWind::_ChangeUsbIoMode(NWind::UsbIoMode mode, bool bCompareCurMode)
{
	MENUITEMINFO			mii = {0};
	BOOL					fRes = FALSE;
	RECT					rc = {0};
	UINT					uMenuItem;

	if (bCompareCurMode) {
		if (m_CurIoMode == mode) return;
	}

	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;

	switch (m_CurIoMode)
	{
	case NWind::UsbIoMod_None:
		uMenuItem = DKM_OPT_NPORT_DISABLE;
		break;

	case NWind::UsbIoMod_SerialIo:
		uMenuItem = DKM_OPT_NPORT_SEL_SERIAL;
		break;

	case NWind::UsbIoMod_VDisplayIo:
		uMenuItem = DKM_OPT_NPORT_SEL_VDISPLAY;
		break;

	default:
		return;
	}
	mii.wID = uMenuItem;
	fRes = GetMenuItemInfo(m_hMenu, uMenuItem, FALSE, &mii);
	if (!fRes) return;

	mii.fMask = MIIM_STATE;
	mii.fState &= ~(MFS_CHECKED);
	fRes = SetMenuItemInfo(m_hMenu, uMenuItem, FALSE, &mii);
	if (!fRes) return;

	bool	bDevRes = false;
	if ((mode == NWind::UsbIoMod_SerialIo) ||
		(mode == NWind::UsbIoMod_VDisplayIo))
	{
		if (m_CurIoMode == NWind::UsbIoMod_None)
		{
			m_NanoOSPort.ReadEvent = true;
			m_NanoOSPort.FormatedReadData = false;
			bDevRes = m_NanoOSPort.EnableAutoDetection(
										m_szNanoOSPortName,
										921600,
										NPort::ByteSize_8,
										NPort::Parity_None,
										NPort::StopBits_1
										);
			if (bDevRes == false)
			{
				mode = NWind::UsbIoMod_None;
			}
		}
	}

	switch (mode)
	{
	case NWind::UsbIoMod_None:
		uMenuItem = DKM_OPT_NPORT_DISABLE;
		break;

	case NWind::UsbIoMod_SerialIo:
		uMenuItem = DKM_OPT_NPORT_SEL_SERIAL;
		break;

	case NWind::UsbIoMod_VDisplayIo:
		uMenuItem = DKM_OPT_NPORT_SEL_VDISPLAY;
		break;

	default:
		return;
	}
	mii.wID = uMenuItem;
	fRes = GetMenuItemInfo(m_hMenu, uMenuItem, FALSE, &mii);
	if (!fRes) return;

	mii.fMask = MIIM_STATE;
	mii.fState |= MFS_CHECKED;
	fRes = SetMenuItemInfo(m_hMenu, uMenuItem, FALSE, &mii);
	if (!fRes) return;

	SetRect(&rc, 0, 0, VDISP_WIDTH, VDISP_HEIGHT);

	switch (mode)
	{
	case NWind::UsbIoMod_None:
		m_VDisp.Title = _T("USB VDisplay [Inactive]");
		m_VDisp.FillRectangle(&rc, RGB(255,255,255));
		m_UsbSerIo.Title = _T("USB Serial IO [Inactive]");
		m_NanoOSPort.DisableAutoDetection();
		break;

	case NWind::UsbIoMod_SerialIo:
		if (m_NanoOSPort.IsOpen) {
			m_UsbSerIo.Title = _T("USB Serial IO - Connected");
		} else {
			m_UsbSerIo.Title = _T("USB Serial IO - Disconnected");
		}
		m_VDisp.Title = _T("USB VDisplay [Inactive]");
		m_VDisp.FillRectangle(&rc, RGB(255,255,255));
		break;

	case NWind::UsbIoMod_VDisplayIo:
		if (m_NanoOSPort.IsOpen) {
			m_VDisp.Title = _T("USB VDisplay - Connected");
		} else {
			m_VDisp.Title = _T("USB VDisplay - Disconnected");
		}
		m_VDisp.FillRectangle(&rc, RGB(0,0,0));
		m_UsbSerIo.Title = _T("USB Serial IO [Inactive]");
		break;

	default:
		return;
	}

	m_CurIoMode = mode;
}

bool NWind::_GetServiceStatus()
{
	bool	bRes = false;
	bool	bIsInstalled = false;
	NSvcControl::SvcStatus	Status;

	bRes = m_SvcCtl.IsInstalled(bIsInstalled);
	if (!bRes) {
		m_Console.AppendText(_T("Error access service!\r\n"));
		return bRes;
	}

	if (!bIsInstalled) {
		m_Console.AppendText(_T("NPortSvc service is not installed.\r\n"));
		return true;
	}

	bRes = m_SvcCtl.GetStatus(Status);
	if (!bRes) {
		m_Console.AppendText(_T("Error getting NPortSvc service status!\r\n"));
		return bRes;
	}

	switch (Status)
	{
	case NSvcControl::Stopped:
		m_Console.AppendText(_T("NPortSvc service is installed and in stopped state.\r\n"));
		break;

	case NSvcControl::StartPending:
		m_Console.AppendText(_T("NPortSvc service is installed and in starting state.\r\n"));
		break;

	case NSvcControl::StopPending:
		m_Console.AppendText(_T("NPortSvc service is installed and in stopping state.\r\n"));
		break;

	case NSvcControl::Running:
		m_Console.AppendText(_T("NPortSvc service is installed and in running state.\r\n"));
		break;

	case NSvcControl::ContinuePending:
		m_Console.AppendText(_T("NPortSvc service is installed and in starting state.\r\n"));
		break;

	case NSvcControl::PuasePending:
		m_Console.AppendText(_T("NPortSvc service is installed and in suspending state.\r\n"));
		break;

	case NSvcControl::Paused:
		m_Console.AppendText(_T("NPortSvc service is installed and in suspended state.\r\n"));
		break;

	default:
		m_Console.AppendText(_T("NPortSvc service is installed but in unknown state.\r\n"));
		break;
	}

	return bRes;
}

bool NWind::_InstallService()
{
	bool	bRes = false;
	bool	bIsInstalled = false;

	bRes = m_SvcCtl.IsInstalled(bIsInstalled);
	if (!bRes) {
		m_Console.AppendText(_T("Error access service!\r\n"));
		return false;
	}

	if (bIsInstalled) {
		m_Console.AppendText(_T("NPortSvc service is already installed.\r\n"));
		return true;
	}

	m_Console.AppendText(_T("Wait... "));
	bRes = m_SvcCtl.Install(m_hWnd, true);
	if (!bRes) {
		if (m_SvcCtl.GetError() == 0) {
			m_Console.AppendText(_T("NPortSvc service installation cancelled!\r\n"));
		} else {
			m_Console.AppendText(_T("Error: Can't install NPortSvc service!\r\n"));
		}
		return false;
	}

	m_Console.AppendText(_T("NPortSvc service has been installed successfully.\r\n"));

	return bRes;
}

bool NWind::_UninstallService()
{
	bool	bRes = false;
	bool	bIsInstalled = false;

	bRes = m_SvcCtl.IsInstalled(bIsInstalled);
	if (!bRes) {
		m_Console.AppendText(_T("Error access service!\r\n"));
		return false;
	}

	if (!bIsInstalled) {
		m_Console.AppendText(_T("NPortSvc service is already uninstalled.\r\n"));
		return true;
	}

	bRes = m_SvcCtl.Uninstall();
	if (!bRes) {
		m_Console.AppendText(_T("Error: Can't uninstall NPortSvc service!\r\n"));
		return false;
	}

	if (m_PrevIoMode != m_CurIoMode) {
		_ChangeUsbIoMode(m_PrevIoMode, true);
	} else {
		if (m_CurIoMode == NWind::UsbIoMod_None) {
			_ChangeUsbIoMode(NWind::UsbIoMod_SerialIo, false);
		}
	}

	m_Console.AppendText(_T("NPortSvc service has been uninstalled successfully.\r\n"));
	return bRes;
}

bool NWind::_StartService()
{
	bool	bRes = false;
	bool	bIsInstalled = false;
	NSvcControl::SvcStatus	Status;

	bRes = m_SvcCtl.IsInstalled(bIsInstalled);
	if (!bRes) {
		m_Console.AppendText(_T("Error access service!\r\n"));
		return bRes;
	}

	if (!bIsInstalled) {
		m_Console.AppendText(_T("NPortSvc service is not installed.\r\n"));
		return true;
	}

	bRes = m_SvcCtl.GetStatus(Status);
	if (!bRes) {
		m_Console.AppendText(_T("Error getting NPortSvc service status!\r\n"));
		return bRes;
	}

	int imb = 0;
	switch (Status)
	{
	case NSvcControl::Stopped:
		imb = MessageBox(
			m_hWnd, _T("Starting NPortSvc service will disable access to NanoOS Port from NTerminal. Do you want to continue ?"),
			_T("Confirmation"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1);
		if (imb == IDNO) break;
		m_PrevIoMode = m_CurIoMode;
		_ChangeUsbIoMode(NWind::UsbIoMod_None, true);
		bRes = m_SvcCtl.Start();
		if (bRes) {
			m_Console.AppendText(_T("Please wait... "));
			Sleep(700);
			bRes = m_SvcCtl.GetStatus(Status);
		}
		if (!bRes) {
			_ChangeUsbIoMode(m_PrevIoMode, true);
			m_Console.AppendText(_T("Error: Can't start NPortSvc service!\r\n"));
		} else {
			if ((Status == NSvcControl::StartPending) ||
				(Status == NSvcControl::Running))
			{
				m_Console.AppendText(_T("NPortSvc service started successfully.\r\n"));
			}
			else
			{
				_ChangeUsbIoMode(m_PrevIoMode, true);
				m_Console.AppendText(_T("Error: Can't start NPortSvc service!\r\n"));
			}
		}
		break;

	case NSvcControl::StartPending:
		m_Console.AppendText(_T("Can't start NPortSvc service: It's in starting state.\r\n"));
		break;

	case NSvcControl::StopPending:
		m_Console.AppendText(_T("Can't start NPortSvc service: It's in stopping state.\r\n"));
		break;

	case NSvcControl::Running:
		m_Console.AppendText(_T("NPortSvc service is already running.\r\n"));
		break;

	case NSvcControl::ContinuePending:
		m_Console.AppendText(_T("Can't start NPortSvc service: It's in starting state.\r\n"));
		break;

	case NSvcControl::PuasePending:
		m_Console.AppendText(_T("Can't start NPortSvc service: It's in suspending state.\r\n"));
		break;

	case NSvcControl::Paused:
		m_Console.AppendText(_T("Can't start NPortSvc service: It's in suspended state.\r\n"));
		break;

	default:
		m_Console.AppendText(_T("Can't start NPortSvc service: It's in unknown state.\r\n"));
		break;
	}

	return bRes;
}

bool NWind::_SuspendService()
{
	bool	bRes = false;
	bool	bIsInstalled = false;
	NSvcControl::SvcStatus	Status;

	bRes = m_SvcCtl.IsInstalled(bIsInstalled);
	if (!bRes) {
		m_Console.AppendText(_T("Error access service!\r\n"));
		return bRes;
	}

	if (!bIsInstalled) {
		m_Console.AppendText(_T("NPortSvc service is not installed.\r\n"));
		return true;
	}

	bRes = m_SvcCtl.GetStatus(Status);
	if (!bRes) {
		m_Console.AppendText(_T("Error getting NPortSvc service status!\r\n"));
		return bRes;
	}

	switch (Status)
	{
	case NSvcControl::Stopped:
		m_Console.AppendText(_T("Can't suspend NPortSvc service: It's in stopped state.\r\n"));
		break;

	case NSvcControl::StartPending:
		m_Console.AppendText(_T("Can't suspend NPortSvc service: It's in starting state.\r\n"));
		break;

	case NSvcControl::StopPending:
		m_Console.AppendText(_T("Can't suspend NPortSvc service: It's in stopping state.\r\n"));
		break;

	case NSvcControl::Running:
		bRes = m_SvcCtl.Pause();
		if (bRes) {
			m_Console.AppendText(_T("NPortSvc service has been suspended successfully.\r\n"));
		} else {
			m_Console.AppendText(_T("Error: Can't suspend NPortSvc service!\r\n"));
		}
		break;

	case NSvcControl::ContinuePending:
		m_Console.AppendText(_T("Can't suspend NPortSvc service: It's in starting state.\r\n"));
		break;

	case NSvcControl::PuasePending:
		m_Console.AppendText(_T("Can't suspend NPortSvc service: It's in suspending state.\r\n"));
		break;

	case NSvcControl::Paused:
		m_Console.AppendText(_T("NPortSvc service is already suspended.\r\n"));
		break;

	default:
		m_Console.AppendText(_T("Can't suspend NPortSvc service: It's in unknown state.\r\n"));
		break;
	}

	return bRes;
}

bool NWind::_ResumeService()
{
	bool	bRes = false;
	bool	bIsInstalled = false;
	NSvcControl::SvcStatus	Status;

	bRes = m_SvcCtl.IsInstalled(bIsInstalled);
	if (!bRes) {
		m_Console.AppendText(_T("Error access service!\r\n"));
		return bRes;
	}

	if (!bIsInstalled) {
		m_Console.AppendText(_T("NPortSvc service is not installed.\r\n"));
		return true;
	}

	bRes = m_SvcCtl.GetStatus(Status);
	if (!bRes) {
		m_Console.AppendText(_T("Error getting NPortSvc service status!\r\n"));
		return bRes;
	}

	switch (Status)
	{
	case NSvcControl::Stopped:
		m_Console.AppendText(_T("Can't resume NPortSvc service: It's in stopped state.\r\n"));
		break;

	case NSvcControl::StartPending:
		m_Console.AppendText(_T("Can't resume NPortSvc service: It's in starting state.\r\n"));
		break;

	case NSvcControl::StopPending:
		m_Console.AppendText(_T("Can't resume NPortSvc service: It's in stopping state.\r\n"));
		break;

	case NSvcControl::Running:
		m_Console.AppendText(_T("Can't resume NPortSvc service: It's in running state.\r\n"));
		break;

	case NSvcControl::ContinuePending:
		m_Console.AppendText(_T("Can't resume NPortSvc service: It's in starting state.\r\n"));
		break;

	case NSvcControl::PuasePending:
		m_Console.AppendText(_T("Can't resume NPortSvc service: It's in suspending state.\r\n"));
		break;

	case NSvcControl::Paused:
		bRes = m_SvcCtl.Resume();
		if (bRes) {
			m_Console.AppendText(_T("NPortSvc service is resummed successfully.\r\n"));
		} else {
			m_Console.AppendText(_T("Error: Can't resume NPortSvc service!\r\n"));
		}
		break;

	default:
		m_Console.AppendText(_T("Can't suspend NPortSvc service: It's in unknown state.\r\n"));
		break;
	}

	return bRes;
}

bool NWind::_StopService()
{
	bool	bRes = false;
	bool	bIsInstalled = false;
	NSvcControl::SvcStatus	Status;

	bRes = m_SvcCtl.IsInstalled(bIsInstalled);
	if (!bRes) {
		m_Console.AppendText(_T("Error access service!\r\n"));
		return bRes;
	}

	if (!bIsInstalled) {
		m_Console.AppendText(_T("NPortSvc service is not installed.\r\n"));
		return true;
	}

	bRes = m_SvcCtl.GetStatus(Status);
	if (!bRes) {
		m_Console.AppendText(_T("Error getting NPortSvc service status!\r\n"));
		return bRes;
	}

	switch (Status)
	{
	case NSvcControl::Stopped:
		m_Console.AppendText(_T("NPortSvc service is already stopped.\r\n"));
		break;

	case NSvcControl::StartPending:
		m_Console.AppendText(_T("Can't stop NPortSvc service: It's in starting state.\r\n"));
		break;

	case NSvcControl::StopPending:
		m_Console.AppendText(_T("Can't stop NPortSvc service: It's in stopping state.\r\n"));
		break;

	case NSvcControl::Running:
		bRes = m_SvcCtl.Stop();
		if (bRes) {
			m_Console.AppendText(_T("Please wait... "));
			Sleep(800);
			bRes = m_SvcCtl.GetStatus(Status);
		}
		if (bRes) {
			if ((Status == NSvcControl::StopPending) ||
				(Status == NSvcControl::Stopped))
			{
				m_Console.AppendText(_T("NPortSvc service stopped successfully.\r\n"));
				_ChangeUsbIoMode(m_PrevIoMode, true);
			}
			else
			{
				m_Console.AppendText(_T("Error: Can't stop NPortSvc service!\r\n"));
			}
		} else {
			m_Console.AppendText(_T("Error: Can't stop NPortSvc service!\r\n"));
		}
		break;

	case NSvcControl::ContinuePending:
		m_Console.AppendText(_T("Can't stop NPortSvc service: It's in starting state.\r\n"));
		break;

	case NSvcControl::PuasePending:
		m_Console.AppendText(_T("Can't stop NPortSvc service: It's in pausing state.\r\n"));
		break;

	case NSvcControl::Paused:
		m_Console.AppendText(_T("Can't stop NPortSvc service: It's in paused state.\r\n"));
		break;

	default:
		m_Console.AppendText(_T("Can't stop NPortSvc service: It's in unknown state.\r\n"));
		break;
	}

	return bRes;
}

void NWind::_HandleOnSvcControlError(const TCHAR *szErr)
{
	m_Console.AppendText(szErr);
}