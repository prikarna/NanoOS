/*
 * File    : Main.cpp
 * Remark  : Sample to test VDisplay (touch screen display emulation) on NanoOS Terminal.
 *
 */

#include "..\NDisplay.h"

using namespace NWindow;

#define NUMB_OF_INPUT		8

extern "C" 
{

struct _INPUT_ITEM {
	UINT8_T			Valid;
	VDISP_IN_DATA	Input;
}__attribute__((packed));

typedef struct _INPUT_ITEM	INPUT_ITEM, *PINPUT_ITEM;

};

INPUT_ITEM		gInputList[NUMB_OF_INPUT];
UINT32_T		guiTh = 0;
Display			gDisp;

void TermMultiThreadedInput(UINT32_T uiTermReason)
{
	if (uiTermReason == TERM__USER_REQUEST) {
		DBG_PRINTF("User termination.\r\n", 0);
	}

	TerminateThread(guiTh);
	CancelReadFromUsbSerial();

	DBG_PRINTF("End termination handler.\r\n", 0);
}

void TermSingleThreadedInput(UINT32_T uiTermReason)
{
	if (uiTermReason == TERM__USER_REQUEST) {
		DBG_PRINTF("User termination.\r\n", 0);
	}

	CancelReadFromUsbSerial();

	DBG_PRINTF("End termination handler.\r\n", 0);
}

int ProcessInput(void *pDat)
{
	UINT32_T	uTID = 0;

	GetCurrentThreadId(&uTID);
	DBG_PRINTF("[%d]Start process input...\r\n", uTID);

	while (TRUE)
	{
		for (int i = 0; i < NUMB_OF_INPUT; i++) {
			if (gInputList[i].Valid == TRUE) {
				switch (gInputList[i].Input.Type)
				{
					case VDISP_IN_TYPE__TOUCH:
						gDisp.DrawCircle(gInputList[i].Input.X, gInputList[i].Input.Y, 4, RGB(255, 255, 0));
						gDisp.DrawCircle(gInputList[i].Input.X, gInputList[i].Input.Y, 8, RGB(255, 255, 0));
						gDisp.DrawCircle(gInputList[i].Input.X, gInputList[i].Input.Y, 12, RGB(255, 255, 0));
						gDisp.DrawCircle(gInputList[i].Input.X, gInputList[i].Input.Y, 4, RGB(0, 0, 0));
						gDisp.DrawCircle(gInputList[i].Input.X, gInputList[i].Input.Y, 8, RGB(0, 0, 0));
						gDisp.DrawCircle(gInputList[i].Input.X, gInputList[i].Input.Y, 12, RGB(0, 0, 0));
						break;

					case VDISP_IN_TYPE__RELEASED:
						break;

					default:
						DBG_PRINTF("Input: Unknown (%d): X=%d, Y=%d\r\n", 
							gInputList[i].Input.Type, gInputList[i].Input.X, gInputList[i].Input.Y);
						break;
				}

				gInputList[i].Valid = FALSE;
			}
		}
	}

	return 0;
}

BOOL ProcessInput2(PVDISP_IN_DATA pInp)
{
	switch (pInp->Type)
	{
		case VDISP_IN_TYPE__TOUCH:
			gDisp.DrawCircle(pInp->X, pInp->Y, 4, RGB(255, 255, 0));
			gDisp.DrawCircle(pInp->X, pInp->Y, 8, RGB(255, 255, 0));
			gDisp.DrawCircle(pInp->X, pInp->Y, 12, RGB(255, 255, 0));
			gDisp.DrawCircle(pInp->X, pInp->Y, 4, RGB(0, 0, 0));
			gDisp.DrawCircle(pInp->X, pInp->Y, 8, RGB(0, 0, 0));
			gDisp.DrawCircle(pInp->X, pInp->Y, 12, RGB(0, 0, 0));
			break;

		case VDISP_IN_TYPE__RELEASED:
			break;

		default:
			DBG_PRINTF("Input: Unknown (%d): X=%d, Y=%d\r\n", pInp->Type, pInp->X, pInp->Y);
			break;
	}

	return TRUE;
}

int main(int argc, char * argv[])
{
	DBG_PRINTF("Test VDisplay device and library.\r\n", 0);

	BOOL	fRes = FALSE;

	DBG_PRINTF("Check USB serial .", 0);
	while (IsUsbSerialReady() == FALSE)
	{
		DBG_PRINTF(".", 0);
		Sleep(500);
	}

	DBG_PRINTF(".ready.\r\n", 0);

	if (!gDisp.Create()) {
		DBG_PRINTF("Fail to create display!\r\n", 0);
		return -1;
	}

	DBG_PRINTF("Paint the display with color of red.\r\n", 0);
	gDisp.Fill(RGB(255,0,0));
	Sleep(500);

	DBG_PRINTF("Paint the display with color of green.\r\n", 0);
	gDisp.Fill(RGB(0,255,0));
	Sleep(500);

	DBG_PRINTF("Paint the display with color of blue.\r\n", 0);
	gDisp.Fill(RGB(0,0,255));
	Sleep(500);

	DBG_PRINTF("Paint the display with color of white.\r\n", 0);
	gDisp.Fill(RGB(255,255,255));
	Sleep(500);

	DBG_PRINTF("Paint the display with color of black.\r\n", 0);
	gDisp.Fill(RGB(0,0,0));
	Sleep(500);

	DBG_PRINTF("Draw lines.\r\n", 0);
	gDisp.DrawLine(5, 5, 100, 5, RGB(255,0,0));
	gDisp.DrawLine(5, 7, 100, 7, RGB(0,255,0));
	gDisp.DrawLine(5, 9, 100, 9, RGB(0,0,255));

	gDisp.DrawLine(5, 12, 5, 200, RGB(255, 127, 127));
	gDisp.DrawLine(7, 12, 7, 200, RGB(127, 255, 127));
	gDisp.DrawLine(9, 12, 9, 200, RGB(127, 127, 255));

	Sleep(500);

	DBG_PRINTF("Draw rectangles.\r\n", 0);
	gDisp.Fill(RGB(0,0,0));
	gDisp.DrawRect(10, 20, 100, 40, RGB(255,96,255));
	gDisp.DrawRect(20, 30, 60, 140, RGB(96,96,255));
	gDisp.DrawRect(40, 100, 100, 200, RGB(255,96,64));

	Sleep(500);

	gDisp.Fill(RGB(0,0,0));
	
	DBG_PRINTF("Draw lines from the center of display, with respect to its direction.\r\n", 0);

	int	iN = 0;
	int	iDiv = VDISP_HEIGHT / 10;
	Point	ptFrom, ptTo;
	ptFrom.x = (VDISP_WIDTH / 2);
	ptFrom.y = (VDISP_HEIGHT / 2);
	ptTo.x = VDISP_WIDTH;
	ptTo.y = 0;

	for (iN = 0; iN < 10; iN++) {
		gDisp.DrawLine(ptFrom, ptTo, RGB(0, 255, 0));
		ptTo.y += iDiv;
	}

	iDiv = VDISP_WIDTH / 10;
	for (iN = 0; iN < 10; iN++) {
		gDisp.DrawLine(ptFrom, ptTo, RGB(0, 0, 255));
		ptTo.x -= iDiv;
	}

	iDiv = VDISP_HEIGHT / 10;
	for (iN = 0; iN < 10; iN++) {
		gDisp.DrawLine(ptFrom, ptTo, RGB(255, 0, 0));
		ptTo.y -= iDiv;
	}

	iDiv = VDISP_WIDTH / 10;
	for (iN = 0; iN < 10; iN++) {
		gDisp.DrawLine(ptFrom, ptTo, RGB(255, 0, 255));
		ptTo.x += iDiv;
	}

	Sleep(500);

	DBG_PRINTF("Draw circles.\r\n", 0);
	gDisp.Fill(RGB(0, 0, 0));
	gDisp.DrawCircle(40, 40, 30, RGB(255, 255, 0));
	gDisp.DrawCircle(ptFrom.x, ptFrom.y, 80, RGB(0, 255, 255));

	Sleep(500);

	DBG_PRINTF("Read pixels.\r\n", 0);
	bool b = false;
	NWindow::Rectangle rc;
	COLORREF cl;
	
	rc.Set(5, 5, 15, 15);
	gDisp.Fill(RGB(255,0,0), rc);
	rc.Offset(15, 0);
	gDisp.Fill(RGB(0,255,0), rc);
	rc.Offset(15, 0);
	gDisp.Fill(RGB(0,0,255), rc);

	for (int x = 5; x < 50; x++)
	{
		b = gDisp.GetPixel(x, 8, cl);
		if (b) {
			DBG_PRINTF("Pixel[%d,8]: Red=%d, Green=%d, Blue=%d\r\n", 
				x, VDISP_GET_RVALUE(cl), VDISP_GET_GVALUE(cl), VDISP_GET_BVALUE(cl));
		} else {
			DBG_PRINTF("Pixel[%d,8]: Error read pixel!\r\n", x);
		}
	}

	Sleep(500);

	gDisp.Fill(RGB(0,0,0));
	DBG_PRINTF("Write text.\r\n", 0);

	const char *szText = 
		"Seekor sapi mengamuk lari dari kandangnya. Si gembala mencoba mengejarnya. "
		"Entah mengapa sampai di ujung jalan tiba-tiba sapi tersebut berhenti. "
		"Namun ketika si gembala mendekati dengan hati-hati, sapi itu lari lagi. "
		"Dan si gembala mengejar kembali.";

	rc.Set(0, 0, VDISP_WIDTH, VDISP_HEIGHT);
	rc.Bottom = 180;
	rc += -5;

	gDisp.m_bMultiLine = true;
	gDisp.m_bWordWrap = true;
	gDisp.SetTextColor(RGB(0,255,0), RGB(0,0,0), false);
	bool bIsNL = false;
	unsigned int iLines = 1;
	b = gDisp.BeginWrite(szText, rc, false);
	while (b) {
		b = gDisp.WriteNextChar(0, 0, 0, bIsNL);
		if ((b) && (bIsNL)) {
			iLines++;
		}
	}
	gDisp.EndWrite();
	DBG_PRINTF("*** Total line(s) being written: %d\r\n", iLines);

	DBG_PRINTF("Write text with center aligment.\r\n", 0);
	const char *szText2 = "Sapi Gila";
	rc.Offset(0, 180);
	rc.Bottom = 300;
	gDisp.m_bMultiLine = false;
	gDisp.m_eTextAlign = TextAlign::Center;
	gDisp.m_iTextScale = 2;
	gDisp.WriteText(szText2, rc);

	gDisp.DrawRect(rc, RGB(0,255,0));

	Sleep(2000);

	gDisp.Fill(RGB(0, 0, 0));
	MemSet((UINT8_PTR_T) &gInputList, 0, sizeof(gInputList));

	DBG_PRINTF("Set termination handler... ", 0);
	fRes = SetTerminationHandler(TermMultiThreadedInput);
	if (fRes) {
		DBG_PRINTF("OK.\r\n", 0);
	} else {
		DBG_PRINTF("failed! (%d)\r\n", GetLastError());
	}

	fRes = CreateThread(THREAD_CONTROL__UNPRIVILEGED, FALSE, ProcessInput, 0, &guiTh);
	if (!fRes) {
		DBG_PRINTF("Fail to create process input thread! (%d)\r\n", GetLastError());
		goto EndProg;
	}

	Sleep(500);
	DBG_PRINTF("Ready to accept input from VDisplay...\r\n", 0);

	while (TRUE) 
	{
		for (int i = 0; i < NUMB_OF_INPUT; i++) {
			if (gInputList[i].Valid == FALSE) {
				fRes = ReadFromUsbSerial((UINT8_PTR_T) &(gInputList[i].Input), sizeof(VDISP_IN_DATA), 0);
				if (fRes) {
					gInputList[i].Valid = TRUE;
				} else {
					break;
				}
			}
		}

		if (!fRes) {
			DBG_PRINTF("Fail to read from USB serial! (%d)\r\n", GetLastError());
			TerminateThread(guiTh);
			break;
		}
	}

	/*
	 * These lines below demonstrate read and handle input in single thread
	 *
	DBG_PRINTF("Set termination handler... ", 0);
	fRes = SetTerminationHandler(TermSingleThreadedInput);
	if (fRes) {
		DBG_PRINTF("OK.\r\n", 0);
	} else {
		DBG_PRINTF("failed! (%d)\r\n", GetLastError());
	}

	DBG_PRINTF("Ready to accept input from VDisplay...\r\n", 0);

	while (TRUE) 
	{
		VDISP_IN_DATA	Input;
		fRes = ReadFromUsbSerial((UINT8_PTR_T) &Input, sizeof(VDISP_IN_DATA), 0);
		if (!fRes) {
			DBG_PRINTF("Fail to get input data! (%d)\r\n", GetLastError());
			break;
		}

		ProcessInput2(&Input);
	}
	*/

EndProg:
	return 0;
}
