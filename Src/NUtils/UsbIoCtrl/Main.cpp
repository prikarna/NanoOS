/*
 *	File	: Main.cpp
 *	Desc.	: Main file of program UsbIoCtrl.
 *			  This program purpose is to access controllers available in STM32103xxx chipset via USB Serial
 *			  IO available on NanoOS. This program is in conjunction with a program called UsbIo in the NanoOS
 *			  solution. UsbIo provide an interface between USB as Serial port to PC and the controllers available
 *			  inside the chipset, so it some short of converter and/or adapter from USB to chipset controllers.
 *			  UsbIo and UsbIsCtrl currently can only access GPIO and USART controllers.
 *
 */

#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"
#include "CommCtrl.h"
#include "ShlwApi.h"

#include "ResId.h"
#include "..\NPortLib\NPortLib.h"
#include "..\..\Application\UsbIo\UsbIo.h"

#pragma comment(lib, "ComCtl32.lib")
#pragma comment(lib, "ShlwApi.lib")

#define		WIND_CLASS_NAME		_T("UsbIoCtrlWind")
#define		WIND_HEIGHT			344
#define		WIND_WIDTH			472

#define		GPIO_TAB_ID			0
#define		USART_TAB_ID		1

HWND		ghWnd = NULL, ghTabs = NULL, ghDisp = NULL;
HFONT		ghDefFont = NULL;

struct Gpio
{
	int	Port;
	int	PinGroup;
	int	StartPin;
	int	SetAs;
	int	InConfig;
	int	OutConfig;
	int	OutMode;
};

struct Usart
{
	int	No;
	int	Baud;
	int	Databits;
	int	Parity;
	int	StopBit;
	int	FlowCtl;
	int	ReadLen;
};

Gpio	SelGpio;
Usart	SelUsart;

IO_REQUEST	gReq;
IO_RESPONSE	gResp;

int IoRequest()
{
	int iRead = 0;
	int	iRes = NPL_Write(reinterpret_cast<unsigned char *>(&gReq), sizeof(IO_REQUEST));
	if (iRes) {
		RtlZeroMemory(&gResp, sizeof(IO_RESPONSE));
		iRes = NPL_Read(reinterpret_cast<unsigned char *>(&gResp), sizeof(IO_RESPONSE), &iRead);
		if ((iRes == 1) && (iRead == 0)) {
			/*
			 * This can also mean 'end of stream', if this occurred we repeat read operation
			 */
			iRes = NPL_Read(reinterpret_cast<unsigned char *>(&gResp), sizeof(IO_RESPONSE), &iRead);
		}
	}

	return iRes;
}

void ChangeSetAsGrp(HWND hDlg)
{
	HWND		hChild;
	LRESULT		lRes;

	hChild = GetDlgItem(hDlg, GPIOID_CONFIG_CB);
	SendMessage(hChild, WM_SETFONT, (WPARAM) ghDefFont, (LPARAM) FALSE);
	do {
		lRes = SendMessage(hChild, CB_DELETESTRING, 0, 0);
	} while (lRes != CB_ERR);
	if (SelGpio.SetAs == 0) {
		SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("Analog"));
		SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("Floating"));
		SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("Pull Up Pull Down"));
		SendMessage(hChild, CB_SETCURSEL, (WPARAM) SelGpio.InConfig, (LPARAM) 0);
	} else {
		SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("Gen. Push Pull"));
		SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("Gen. Open Drain"));
		SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("Alt. Push Pull"));
		SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("Alt. Open Drain"));
		SendMessage(hChild, CB_SETCURSEL, (WPARAM) SelGpio.OutConfig, (LPARAM) 0);
	}

	hChild = GetDlgItem(hDlg, GPIOID_OUTMODE_CB);
	SendMessage(hChild, WM_SETFONT, (WPARAM) ghDefFont, (LPARAM) FALSE);
	do {
		lRes = SendMessage(hChild, CB_DELETESTRING, 0, 0);
	} while (lRes != CB_ERR);
	if (SelGpio.SetAs == 0) {
		EnableWindow(hChild, FALSE);
	} else {
		SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("10 MHz"));
		SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("2 MHz"));
		SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("50 MHz"));
		SendMessage(hChild, CB_SETCURSEL, (WPARAM) SelGpio.OutMode, (LPARAM) 0);

		EnableWindow(hChild, TRUE);
	}
}

void OnGpioDlgInit(HWND hDlg)
{
	HWND	hChild;
	TCHAR	szBuf[64];

	hChild = GetDlgItem(hDlg, GPIOID_PORT_CB);
	SendMessage(hChild, WM_SETFONT, (WPARAM) ghDefFont, (LPARAM) FALSE);
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("A"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("B"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("C"));
	//SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("D"));
	SendMessage(hChild, CB_SETCURSEL, (WPARAM) SelGpio.Port, (LPARAM) 0);

	hChild = GetDlgItem(hDlg, GPIOID_PIN_GRP_CB);
	SendMessage(hChild, WM_SETFONT, (WPARAM) ghDefFont, (LPARAM) FALSE);
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("Single"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("FourPins"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("EightPins"));
	SendMessage(hChild, CB_SETCURSEL, (WPARAM) SelGpio.PinGroup, (LPARAM) 0);

	hChild = GetDlgItem(hDlg, GPIOID_PIN_CB);
	SendMessage(hChild, WM_SETFONT, (WPARAM) ghDefFont, (LPARAM) FALSE);
	for (int i = 0; i < 16; i++) {
		RtlZeroMemory(szBuf, sizeof(szBuf));
		StringCbPrintf(szBuf, sizeof(szBuf), _T("%d"), i);
		SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) szBuf);
	}
	SendMessage(hChild, CB_SETCURSEL, (WPARAM) SelGpio.StartPin, (LPARAM) 0);

	hChild = GetDlgItem(hDlg, GPIOID_SETAS_CB);
	SendMessage(hChild, WM_SETFONT, (WPARAM) ghDefFont, (LPARAM) FALSE);
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("Input"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("Output"));
	if (SelGpio.SetAs == 0) {
		SendMessage(hChild, CB_SETCURSEL, (WPARAM) 0, (LPARAM) 0);
	} else {
		SendMessage(hChild, CB_SETCURSEL, (WPARAM) 1, (LPARAM) 0);
	}

	ChangeSetAsGrp(hDlg);

	hChild = GetDlgItem(hDlg, GPIOID_READ_EDT);
	SendMessage(hChild, EM_SETLIMITTEXT, 3, 0);

	hChild = GetDlgItem(hDlg, GPIOID_WRITE_EDT);
	SendMessage(hChild, EM_SETLIMITTEXT, 3, 0);
}

bool SetGpioReq(unsigned char ucGpioCmd)
{
	gReq.Type	= IOTYPE_GPIO;
	gReq.Request.Gpio.Address.Port = static_cast<unsigned char>(SelGpio.Port);
	switch (SelGpio.PinGroup)
	{
	case 0:
		gReq.Request.Gpio.Address.PinGroup = GPIOGRP_SINGLE_PIN;
		break;

	case 1:
		gReq.Request.Gpio.Address.PinGroup = GPIOGRP_FOUR_PINS;
		break;

	case 2:
		gReq.Request.Gpio.Address.PinGroup = GPIOGRP_EIGHT_PINS;
		break;

	default:
		return false;
	}
	gReq.Request.Gpio.Address.Pin = static_cast<unsigned char>(SelGpio.StartPin);

	gReq.Request.Gpio.Cmd = ucGpioCmd;

	return true;
}

bool SetGpioPinAs()
{
	bool	b = SetGpioReq(GPIOCMD_SET_AS);
	if (!b) {
		MessageBox(ghWnd, _T("Can't set gpio request!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	gReq.Request.Gpio.u.SetAs.Type = static_cast<unsigned char>(SelGpio.SetAs);
	if (SelGpio.SetAs == GPIOSETASTYPE_INPUT) {
		switch (SelGpio.InConfig)
		{
		case 0:
			gReq.Request.Gpio.u.SetAs.InputConfig = GPIOINCFG_ANALOG;
			break;

		case 1:
			gReq.Request.Gpio.u.SetAs.InputConfig = GPIOINCFG_FLOATING;
			break;

		case 2:
			gReq.Request.Gpio.u.SetAs.InputConfig = GPIOINCFG_PULL_UP_PULL_DOWN;
			break;

		default:
			MessageBox(ghWnd, _T("Invalid input config!"), _T("Error"), MB_OK | MB_ICONERROR);
			return false;
		}
	} else {
		switch (SelGpio.OutConfig)
		{
		case 0:
			gReq.Request.Gpio.u.SetAs.OutputConfig = GPIOOUTCFG_GEN_PUSH_PULL;
			break;

		case 1:
			gReq.Request.Gpio.u.SetAs.OutputConfig = GPIOOUTCFG_GEN_OPEN_DRAIN;
			break;

		case 2:
			gReq.Request.Gpio.u.SetAs.OutputConfig = GPIOOUTCFG_ALT_PUSH_PULL;
			break;

		case 3:
			gReq.Request.Gpio.u.SetAs.OutputConfig = GPIOOUTCFG_ALT_OPEN_DRAIN;
			break;

		default:
			MessageBox(ghWnd, _T("Invalid output config!"), _T("Error"), MB_OK | MB_ICONERROR);
			return false;
		}

		switch (SelGpio.OutMode)
		{
		case 0:
			gReq.Request.Gpio.u.SetAs.OutputMode = GPIOOUTMOD_10_MHZ;
			break;

		case 1:
			gReq.Request.Gpio.u.SetAs.OutputMode = GPIOOUTMOD_2_MHZ;
			break;

		case 2:
			gReq.Request.Gpio.u.SetAs.OutputMode = GPIOOUTMOD_50_MHZ;
			break;

		default:
			MessageBox(ghWnd, _T("Invalid output mode!"), _T("Error"), MB_OK | MB_ICONERROR);
			return false;
		}
	}

	int iRes = IoRequest();
	if (iRes == 0) {
		MessageBox(ghWnd, _T("Request error!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	if (gResp.Result == IORESP_OK) {
		return true;
	}

	MessageBox(ghWnd, _T("Fail to set pin!"), _T("Error"), MB_OK | MB_ICONERROR);
	return false;
}

bool ReadGpioPin(HWND hDlg)
{
	bool	b = SetGpioReq(GPIOCMD_READ);
	if (!b) {
		MessageBox(ghWnd, _T("Can't set gpio request!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	int iRes = IoRequest();
	if (iRes == 0) {
		MessageBox(ghWnd, _T("Request error!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	if (gResp.Result == IORESP_OK) {
		char szBuf[64];
		RtlZeroMemory(&szBuf, sizeof(szBuf));
		StringCbPrintfA(szBuf, sizeof(szBuf), "%d", gResp.Data[0]);
		SetWindowTextA(GetDlgItem(hDlg, GPIOID_READ_EDT), szBuf);
		return true;
	}

	MessageBox(ghWnd, _T("Fail to read pin!"), _T("Error"), MB_OK | MB_ICONERROR);
	return false;
}

bool WriteGpioPin(HWND hDlg)
{
	TCHAR szBuf[64];
	RtlZeroMemory(&szBuf, sizeof(szBuf));
	int i = GetWindowText(GetDlgItem(hDlg, GPIOID_WRITE_EDT), szBuf, sizeof(szBuf)/sizeof(TCHAR));
	if (i <= 0) {
		MessageBox(ghWnd, _T("Can't get text to write to!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	BOOL f = StrToIntEx(szBuf, STIF_DEFAULT, &i);
	if (!f) {
		MessageBox(ghWnd, _T("Can't convert string to int!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	if ((i > 255) || (i < 0)) {
		MessageBox(ghWnd, _T("Write value must be in the range of 0 to 255!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	if (!SetGpioReq(GPIOCMD_WRITE)) {
		MessageBox(ghWnd, _T("Can't set gpio request!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	gReq.Request.Gpio.u.Write.Data = static_cast<unsigned char>(i);
	i = IoRequest();
	if (i <= 0) {
		MessageBox(ghWnd, _T("Request error!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	if (gResp.Result == IORESP_OK)
		return true;

	MessageBox(ghWnd, _T("Fail to write pin!"), _T("Error"), MB_OK | MB_ICONERROR);
	return false;
}

INT_PTR CALLBACK GPIOProc(HWND hDlg, UINT uMsg, WPARAM wParm, LPARAM lParm)
{
	INT_PTR	iRes = FALSE;
	LRESULT	lRes;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		OnGpioDlgInit(hDlg);
		iRes = TRUE;
		break;

	case WM_COMMAND:
		if (HIWORD(wParm) == CBN_SELCHANGE)
		{
			switch (LOWORD(wParm))
			{
			case GPIOID_PORT_CB:
				SelGpio.Port = SendMessage(GetDlgItem(hDlg, GPIOID_PORT_CB), CB_GETCURSEL, 0, 0);
				iRes = TRUE;
				break;

			case GPIOID_PIN_GRP_CB:
				SelGpio.PinGroup = SendMessage(GetDlgItem(hDlg, GPIOID_PIN_GRP_CB), CB_GETCURSEL, 0, 0);
				iRes = TRUE;
				break;

			case GPIOID_PIN_CB:
				SelGpio.StartPin = SendMessage(GetDlgItem(hDlg, GPIOID_PIN_CB), CB_GETCURSEL, 0, 0);
				iRes = TRUE;
				break;

			case GPIOID_SETAS_CB:
				lRes = SendMessage(GetDlgItem(hDlg, GPIOID_SETAS_CB), CB_GETCURSEL, 0, 0);
				SelGpio.SetAs = lRes;
				ChangeSetAsGrp(hDlg);
				iRes = TRUE;
				break;

			case GPIOID_CONFIG_CB:
				if (SelGpio.SetAs == 0) {
					SelGpio.InConfig = SendMessage(GetDlgItem(hDlg, GPIOID_CONFIG_CB), CB_GETCURSEL, 0, 0);
				} else {
					SelGpio.OutConfig = SendMessage(GetDlgItem(hDlg, GPIOID_CONFIG_CB), CB_GETCURSEL, 0, 0);
				}
				iRes = TRUE;
				break;

			case GPIOID_OUTMODE_CB:
				SelGpio.OutMode = SendMessage(GetDlgItem(hDlg, GPIOID_OUTMODE_CB), CB_GETCURSEL, 0, 0);
				iRes = TRUE;
				break;

			default:
				break;
			}
		}
		else if (HIWORD(wParm) == BN_CLICKED)
		{
			switch (LOWORD(wParm))
			{
			case GPIOID_SET_PIN_BTN:
				SetGpioPinAs();
				break;

			case GPIOID_READ_BTN:
				ReadGpioPin(hDlg);
				break;

			case GPIOID_WRITE_BTN:
				WriteGpioPin(hDlg);
				break;

			default:
				break;
			}
		}
		else
		{
		}
		break;

	default:
		break;
	}

	return iRes;
}

void OnUsartDlgInit(HWND hDlg)
{
	HWND	hChild;

	hChild = GetDlgItem(hDlg, USARTID_NO_CB);
	SendMessage(hChild, WM_SETFONT, (WPARAM) ghDefFont, (LPARAM) FALSE);
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("1"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("2"));
	SendMessage(hChild, CB_SETCURSEL, (WPARAM) SelUsart.No, (LPARAM) 0);

	hChild = GetDlgItem(hDlg, USARTID_BAUD_CB);
	SendMessage(hChild, WM_SETFONT, (WPARAM) ghDefFont, (LPARAM) FALSE);
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("None"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("2400"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("9600"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("19200"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("57600"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("115200"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("230400"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("460800"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("921600"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("2250000"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("4500000"));
	SendMessage(hChild, CB_SETCURSEL, (WPARAM) SelUsart.Baud, (LPARAM) 0);

	hChild = GetDlgItem(hDlg, USARTID_DATABITS_CB);
	SendMessage(hChild, WM_SETFONT, (WPARAM) ghDefFont, (LPARAM) FALSE);
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("8"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("9"));
	SendMessage(hChild, CB_SETCURSEL, (WPARAM) SelUsart.Databits, (LPARAM) 0);

	hChild = GetDlgItem(hDlg, USARTID_PARITY_CB);
	SendMessage(hChild, WM_SETFONT, (WPARAM) ghDefFont, (LPARAM) FALSE);
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("None"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("Odd"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("Even"));
	SendMessage(hChild, CB_SETCURSEL, (WPARAM) SelUsart.No, (LPARAM) 0);

	hChild = GetDlgItem(hDlg, USARTID_STOPBIT_CB);
	SendMessage(hChild, WM_SETFONT, (WPARAM) ghDefFont, (LPARAM) FALSE);
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("1"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("0.5"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("1.5"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("2"));
	SendMessage(hChild, CB_SETCURSEL, (WPARAM) SelUsart.StopBit, (LPARAM) 0);

	hChild = GetDlgItem(hDlg, USARTID_FLOWCTL_CB);
	SendMessage(hChild, WM_SETFONT, (WPARAM) ghDefFont, (LPARAM) FALSE);
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("None"));
	SendMessage(hChild, CB_INSERTSTRING, (WPARAM) -1, (LPARAM) _T("Hardware"));
	SendMessage(hChild, CB_SETCURSEL, (WPARAM) SelUsart.StopBit, (LPARAM) 0);

	hChild = GetDlgItem(hDlg, USARTID_READ_LEN_EDT);
	SendMessage(hChild, EM_SETLIMITTEXT, 2, 0);
	TCHAR szBuf[64];
	RtlZeroMemory(szBuf, sizeof(szBuf));
	StringCbPrintf(szBuf, sizeof(szBuf), _T("%d"), SelUsart.ReadLen);
	SetWindowText(hChild, szBuf);

	hChild = GetDlgItem(hDlg, USARTID_WRITE_EDT);
	SendMessage(hChild, EM_SETLIMITTEXT, 64, 0);
}

bool SetUsartRequest(unsigned char ucCmd)
{
	RtlZeroMemory(&gReq, sizeof(IO_REQUEST));
	gReq.Type = IOTYPE_USART;
	gReq.Request.Usart.Address = static_cast<unsigned char>(SelUsart.No);

	if ((ucCmd == USARTCMD_OPEN) ||
		(ucCmd == USARTCMD_READ) ||
		(ucCmd == USARTCMD_WRITE) ||
		(ucCmd == USARTCMD_CLOSE))
	{
		gReq.Request.Usart.Cmd = ucCmd;
		return true;
	}

	return false;
}

bool OpenUsart()
{
	if (!SetUsartRequest(USARTCMD_OPEN)) { 
		MessageBox(ghWnd, _T("Can't set usart request!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	switch (SelUsart.Baud)
	{
	case 0:
		MessageBox(ghWnd, _T("Invalid usart baud rate!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;

	case 1:
		gReq.Request.Usart.u.Open.BaudRate = USARTBAUD_2400;
		break;

	case 2:
		gReq.Request.Usart.u.Open.BaudRate = USARTBAUD_9600;
		break;

	case 3:
		gReq.Request.Usart.u.Open.BaudRate = USARTBAUD_19200;
		break;

	case 4:
		gReq.Request.Usart.u.Open.BaudRate = USARTBAUD_57600;
		break;

	case 5:
		gReq.Request.Usart.u.Open.BaudRate = USARTBAUD_115200;
		break;

	case 6:
		gReq.Request.Usart.u.Open.BaudRate = USARTBAUD_230400;
		break;

	case 7:
		gReq.Request.Usart.u.Open.BaudRate = USARTBAUD_460800;
		break;

	case 8:
		gReq.Request.Usart.u.Open.BaudRate = USARTBAUD_921600;
		break;

	case 9:
		gReq.Request.Usart.u.Open.BaudRate = USARTBAUD_2250000;
		break;

	case 10:
		gReq.Request.Usart.u.Open.BaudRate = USARTBAUD_4500000;
		break;

	default:
		MessageBox(ghWnd, _T("Invalid usart baud rate!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	gReq.Request.Usart.u.Open.DataBits = static_cast<unsigned char>(SelUsart.Databits);
	gReq.Request.Usart.u.Open.FlowControl = static_cast<unsigned char>(SelUsart.FlowCtl);
	gReq.Request.Usart.u.Open.Parity = static_cast<unsigned char>(SelUsart.Parity);
	gReq.Request.Usart.u.Open.StopBit = static_cast<unsigned char>(SelUsart.StopBit);

	int	iRes = IoRequest();
	if (iRes <= 0) {
		MessageBox(ghWnd, _T("Request error!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	if (gResp.Result == IORESP_OK)
		return true;

	MessageBox(ghWnd, _T("Fail to open usart!"), _T("Error"), MB_OK | MB_ICONERROR);
	return false;
}

bool CloseUsart()
{
	SetUsartRequest(USARTCMD_CLOSE);
	
	int iRes = IoRequest();
	if (iRes <= 0) {
		MessageBox(ghWnd, _T("Request error!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	if (gResp.Result == IORESP_OK) 
		return true;

	MessageBox(ghWnd, _T("Fail to close usart!"), _T("Error"), MB_OK | MB_ICONERROR);
	return false;
}

/* 
 * Note this is 'blocking' or 'synchronous' operation, which will not return until all data 
 * at specified length has successfully being read or an error occured.
 */
bool ReadUsart(HWND hDlg)
{
	char szBuf[MAX_IO_DATA + 1];
	RtlZeroMemory(szBuf, sizeof(szBuf));

	int iRes = GetWindowTextA(GetDlgItem(hDlg, USARTID_READ_LEN_EDT), szBuf, sizeof(szBuf));
	if (iRes <= 0) {
		MessageBox(ghWnd, _T("Can't get read length!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	BOOL f = StrToIntExA(szBuf, STIF_DEFAULT, &iRes);
	if (!f) {
		MessageBox(ghWnd, _T("Can't convert string to int!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	if ((iRes <= 0) || (iRes > MAX_IO_DATA)) {
		MessageBox(ghWnd, _T("Read length must be in the range of 1 to 48!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	SetUsartRequest(USARTCMD_READ);

	gReq.Request.Usart.u.Read.Length = static_cast<unsigned char>(iRes);

	iRes = IoRequest();
	if (iRes <= 0) {
		MessageBox(ghWnd, _T("Request error!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	if (gResp.Result == IORESP_OK)
	{
		RtlZeroMemory(szBuf, sizeof(szBuf));
		RtlCopyMemory(szBuf, &(gResp.Data[0]), gResp.Length);
		SetWindowTextA(GetDlgItem(hDlg, USARTID_READ_EDT), szBuf);
		return true;
	}

	MessageBox(ghWnd, _T("Fail to read usart!"), _T("Error"), MB_OK | MB_ICONERROR);
	return false;
}

bool WriteUsart(HWND hDlg)
{
	char	szBuf[MAX_IO_DATA + 1] = {0};

	GetWindowTextA(GetDlgItem(hDlg, USARTID_WRITE_EDT), szBuf, sizeof(szBuf));
	int iRes = lstrlenA(szBuf);
	if ((iRes <= 0) || (iRes > MAX_IO_DATA)) {
		MessageBox(ghWnd, _T("Can't write empty string!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	SetUsartRequest(USARTCMD_WRITE);
	gReq.Request.Usart.u.Write.Length = static_cast<unsigned char>(iRes);
	RtlCopyMemory(&(gReq.Request.Usart.u.Write.Data[0]), szBuf, iRes);

	iRes = IoRequest();
	if (iRes <= 0) {
		MessageBox(ghWnd, _T("Request error!"), _T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	if (gResp.Result == IORESP_OK) {
		return true;
	}

	MessageBox(ghWnd, _T("Fail to write usart!"), _T("Error"), MB_OK | MB_ICONERROR);
	return false;
}

INT_PTR CALLBACK USARTProc(HWND hDlg, UINT uMsg, WPARAM wParm, LPARAM lParm)
{
	INT_PTR	iRes = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		OnUsartDlgInit(hDlg);
		iRes = TRUE;
		break;

	case WM_COMMAND:
		if (HIWORD(wParm) == CBN_SELCHANGE)
		{
			switch (LOWORD(wParm))
			{
			case USARTID_NO_CB:
				SelUsart.No = SendMessage(GetDlgItem(hDlg, USARTID_NO_CB), CB_GETCURSEL, 0, 0);
				iRes = TRUE;
				break;

			case USARTID_BAUD_CB:
				SelUsart.Baud = SendMessage(GetDlgItem(hDlg, USARTID_BAUD_CB), CB_GETCURSEL, 0, 0);
				iRes = TRUE;
				break;

			case USARTID_DATABITS_CB:
				SelUsart.Databits = SendMessage(GetDlgItem(hDlg, USARTID_DATABITS_CB), CB_GETCURSEL, 0, 0);
				iRes = TRUE;
				break;

			case USARTID_PARITY_CB:
				SelUsart.Parity = SendMessage(GetDlgItem(hDlg, USARTID_PARITY_CB), CB_GETCURSEL, 0, 0);
				iRes = TRUE;
				break;

			case USARTID_STOPBIT_CB:
				SelUsart.StopBit = SendMessage(GetDlgItem(hDlg, USARTID_STOPBIT_CB), CB_GETCURSEL, 0, 0);
				iRes = TRUE;
				break;

			case USARTID_FLOWCTL_CB:
				SelUsart.FlowCtl = SendMessage(GetDlgItem(hDlg, USARTID_FLOWCTL_CB), CB_GETCURSEL, 0, 0);
				iRes = TRUE;
				break;

			default:
				break;
			}
		}
		else if (HIWORD(wParm) == BN_CLICKED)
		{
			switch (LOWORD(wParm))
			{
			case USARTID_OPEN_BTN:
				OpenUsart();
				iRes = TRUE;
				break;

			case USARTID_CLOSE_BTN:
				CloseUsart();
				iRes = TRUE;
				break;

			case USARTID_READ_BTN:
				ReadUsart(hDlg);
				iRes = TRUE;
				break;

			case USARTID_WRITE_BTN:
				WriteUsart(hDlg);
				iRes = TRUE;
				break;

			default:
				break;
			}
		}
		else
		{
		}
		break;

	default:
		break;
	}

	return iRes;
}

LRESULT CALLBACK WProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm)
{
	HINSTANCE	hInst;
	RECT		rc;

	switch (uMsg)
	{
	case WM_CLOSE:
		NPL_Close();
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParm)->code == TCN_SELCHANGE)
		{
			int	iPage = TabCtrl_GetCurSel(ghTabs);
			if (iPage == GPIO_TAB_ID) {
				if (ghDisp) {
					DestroyWindow(ghDisp);
				}
				hInst = (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
				ghDisp = CreateDialog(hInst, MAKEINTRESOURCE(IDD_GPIODLG), ghTabs, GPIOProc);
				if (ghDisp) {
					GetClientRect(ghTabs, &rc);
					TabCtrl_AdjustRect(ghTabs, FALSE, &rc);
					MoveWindow(ghDisp, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), TRUE);
					ShowWindow(ghDisp, SW_SHOW);
				}
			} else if (iPage == USART_TAB_ID) {
				if (ghDisp) {
					DestroyWindow(ghDisp);
				}
				hInst = (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
				ghDisp = CreateDialog(hInst, MAKEINTRESOURCE(IDD_USARTDLG), ghTabs, USARTProc);
				if (ghDisp) {
					GetClientRect(ghTabs, &rc);
					TabCtrl_AdjustRect(ghTabs, FALSE, &rc);
					MoveWindow(ghDisp, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), TRUE);
					ShowWindow(ghDisp, SW_SHOW);
				}
			} else {
				MessageBox(hWnd, _T("Unknown tab id"), _T("Info"), MB_OK);
			}
			return static_cast<LRESULT>(TRUE);
		}
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParm, lParm);
	}

	return static_cast<LRESULT>(FALSE);
}

void CALLBACK HandleOnConnChanged(unsigned char fConnected)
{
	if (fConnected) {
		SetWindowText(ghWnd, _T("UsbIo Control Panel - Connected"));
	} else {
		SetWindowText(ghWnd, _T("UsbIo Control Panel - Disconnected"));
	}
}

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE , LPTSTR , int iCmdShow)
{
	SelUsart.ReadLen = 1;

	WNDCLASSEX	wcx = {0};

	wcx.cbSize			= sizeof(WNDCLASSEX);
	wcx.hbrBackground	= reinterpret_cast<HBRUSH>(COLOR_WINDOW);
	wcx.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wcx.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wcx.hInstance		= hInst;
	wcx.lpfnWndProc		= reinterpret_cast<WNDPROC>(WProc);
	wcx.lpszClassName	= WIND_CLASS_NAME;
	wcx.lpszMenuName	= NULL;
	wcx.style			= CS_HREDRAW | CS_VREDRAW;

	ATOM at;
	at = RegisterClassEx(&wcx);
	if (at <= 0){
		MessageBox(NULL, _T("Error register window class!"), _T("Error"), MB_OK | MB_ICONERROR);
		return -1;
	}

	int	iScX = GetSystemMetrics(SM_CXFULLSCREEN);
	int iScY = GetSystemMetrics(SM_CYFULLSCREEN);
	int iXPos = (iScX - WIND_WIDTH) / 2;
	int iYPos = (iScY - WIND_HEIGHT) / 2;

	RECT	rc;
	DWORD	dwStyles = WS_CAPTION | WS_BORDER | WS_SYSMENU;
	SetRect(&rc, iXPos, iYPos, (WIND_WIDTH + iXPos), (WIND_HEIGHT + iYPos));
	AdjustWindowRectEx(&rc, dwStyles, FALSE, 0);

	ghWnd = CreateWindowEx(
						0,
						WIND_CLASS_NAME,
						_T("UsbIo Control Panel - Disconnected"),
						dwStyles,
						rc.left,
						rc.top,
						WIND_WIDTH,
						WIND_HEIGHT,
						NULL,
						NULL,
						hInst,
						NULL
						);
	if (!ghWnd) {
		MessageBox(NULL, _T("Error create window!"), _T("Error"), MB_OK | MB_ICONERROR);
		UnregisterClass(WIND_CLASS_NAME, hInst);
		return -2;
	}

	ghDefFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

	INITCOMMONCONTROLSEX	ccx = {0};
	ccx.dwICC		= ICC_TAB_CLASSES | 
					  ICC_STANDARD_CLASSES;
	ccx.dwSize		= sizeof(INITCOMMONCONTROLSEX);

	InitCommonControlsEx(&ccx);

	GetClientRect(ghWnd, &rc);
	ghTabs = CreateWindowEx(
					0,
					WC_TABCONTROL,
					NULL,
					WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_VERTICAL | TCS_MULTILINE | TCS_RIGHT,
					0,
					0,
					rc.right,
					rc.bottom,
					ghWnd,
					NULL,
					hInst,
					NULL
					);
	if (ghTabs) 
	{
		TCITEM	tci;
		RtlZeroMemory(&tci, sizeof(TCITEM));

		tci.mask	= TCIF_TEXT;
		tci.pszText	= _T("GPIO");
		TabCtrl_InsertItem(ghTabs, GPIO_TAB_ID, &tci);

		tci.pszText = _T("USART");
		TabCtrl_InsertItem(ghTabs, USART_TAB_ID, &tci);

		ghDisp = CreateDialog(hInst, MAKEINTRESOURCE(IDD_GPIODLG), ghTabs, GPIOProc);
		if (ghDisp) {
			GetClientRect(ghTabs, &rc);
			TabCtrl_AdjustRect(ghTabs, FALSE, &rc);
			MoveWindow(ghDisp, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), TRUE);
			ShowWindow(ghDisp, SW_SHOW);
		}
	}

	int iRes = NPL_Open(HandleOnConnChanged, 0, 0);

	ShowWindow(ghWnd, iCmdShow);
	UpdateWindow(ghWnd);

	MSG	msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClass(WIND_CLASS_NAME, hInst);

	return static_cast<int>(msg.wParam);
}