/*
 * File    : VDispTest.c
 * Remark  : Sample to test VDisplay (touch screen display emulation) on NanoOS Terminal.
 *
 */

#include "..\NanoOSApi.h"
#include "..\VDispLib.h"

#define NUMB_OF_INPUT		8

struct _INPUT_ITEM {
	UINT8_T			Valid;
	VDISP_IN_DATA	Input;
}__attribute__((packed));

typedef struct _INPUT_ITEM	INPUT_ITEM, *PINPUT_ITEM;

INPUT_ITEM		gInputList[NUMB_OF_INPUT];
UINT32_T		guiTh = 0;

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
						DrawCircle(gInputList[i].Input.X, gInputList[i].Input.Y, 4, 255, 255, 0);
						DrawCircle(gInputList[i].Input.X, gInputList[i].Input.Y, 8, 255, 255, 0);
						DrawCircle(gInputList[i].Input.X, gInputList[i].Input.Y, 12, 255, 255, 0);
						DrawCircle(gInputList[i].Input.X, gInputList[i].Input.Y, 4, 0, 0, 0);
						DrawCircle(gInputList[i].Input.X, gInputList[i].Input.Y, 8, 0, 0, 0);
						DrawCircle(gInputList[i].Input.X, gInputList[i].Input.Y, 12, 0, 0, 0);
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
			DrawCircle(pInp->X, pInp->Y, 4, 255, 255, 0);
			DrawCircle(pInp->X, pInp->Y, 8, 255, 255, 0);
			DrawCircle(pInp->X, pInp->Y, 12, 255, 255, 0);
			DrawCircle(pInp->X, pInp->Y, 4, 0, 0, 0);
			DrawCircle(pInp->X, pInp->Y, 8, 0, 0, 0);
			DrawCircle(pInp->X, pInp->Y, 12, 0, 0, 0);
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

	DBG_PRINTF("Paint the display with color of red.\r\n", 0);
	FillRectangle(0, 0, VDISP_WIDTH, VDISP_HEIGHT, 255, 0, 0);
	Sleep(500);

	DBG_PRINTF("Paint the display with color of green.\r\n", 0);
	FillRectangle(0, 0, VDISP_WIDTH, VDISP_HEIGHT, 0, 255, 0);
	Sleep(500);

	DBG_PRINTF("Paint the display with color of blue.\r\n", 0);
	FillRectangle(0, 0, VDISP_WIDTH, VDISP_HEIGHT, 0, 0, 255);
	Sleep(500);

	DBG_PRINTF("Paint the display with color of white.\r\n", 0);
	FillRectangle(0, 0, VDISP_WIDTH, VDISP_HEIGHT, 255, 255, 255);
	Sleep(500);

	DBG_PRINTF("Paint the display with color of black.\r\n", 0);
	FillRectangle(0, 0, VDISP_WIDTH, VDISP_HEIGHT, 0, 0, 0);
	Sleep(500);

	DBG_PRINTF("Draw lines.\r\n", 0);
	DrawHorizontalLine(5, 5, 100, 255, 0, 0);
	//UpdateDisplay();
	DrawHorizontalLine(5, 7, 100, 0, 255, 0);
	//UpdateDisplay();
	DrawHorizontalLine(5, 9, 100, 0, 0, 255);
	//UpdateDisplay();

	DrawVerticalLine(5, 12, 200, 255, 127, 127);
	//UpdateDisplay();
	DrawVerticalLine(7, 12, 200, 127, 255, 127);
	//UpdateDisplay();
	DrawVerticalLine(9, 12, 200, 127, 127, 255);
	//UpdateDisplay();

	Sleep(500);

	DBG_PRINTF("Draw rectangles.\r\n", 0);
	FillRectangle(0, 0, VDISP_WIDTH, VDISP_HEIGHT, 0, 0, 0);
	DrawRect(10, 20, 100, 40, 255, 96, 255);
	//UpdateDisplay();
	DrawRect(20, 30, 60, 140, 96, 96, 255);
	//UpdateDisplay();
	DrawRect(40, 100, 100, 200, 255, 96, 64);
	//UpdateDisplay();

	Sleep(500);

	FillRectangle(0, 0, VDISP_WIDTH, VDISP_HEIGHT, 0, 0, 0);
	
	DBG_PRINTF("Draw lines from the center of display.\r\n", 0);

	UINT32_T	uN = 0;
	UINT32_T	uDiv = VDISP_HEIGHT / 10;
	VDISP_POINT	ptFrom, ptTo;
	ptFrom.X = (VDISP_WIDTH / 2);
	ptFrom.Y	= (VDISP_HEIGHT / 2);
	ptTo.X = VDISP_WIDTH;
	ptTo.Y = 0;

	for (uN = 0; uN < 10; uN++) {
		DrawLine(&ptFrom, &ptTo, 0, 255, 0);
		ptTo.Y += uDiv;
	}

	uDiv = VDISP_WIDTH / 10;
	for (uN = 0; uN < 10; uN++) {
		DrawLine(&ptFrom, &ptTo, 0, 0, 255);
		ptTo.X -= uDiv;
	}

	uDiv = VDISP_HEIGHT / 10;
	for (uN = 0; uN < 10; uN++) {
		DrawLine(&ptFrom, &ptTo, 255, 0, 0);
		ptTo.Y -= uDiv;
	}

	uDiv = VDISP_WIDTH / 10;
	for (uN = 0; uN < 10; uN++) {
		DrawLine(&ptFrom, &ptTo, 255, 0, 255);
		ptTo.X += uDiv;
	}

	Sleep(500);

	DBG_PRINTF("Draw circles.\r\n", 0);
	FillRectangle(0, 0, VDISP_WIDTH, VDISP_HEIGHT, 0, 0, 0);
	DrawCircle(40, 40, 30, 255, 255, 0);
	DrawCircle(ptFrom.X, ptFrom.Y, 80, 0, 255, 255);

	Sleep(500);

	FillRectangle(0, 0, VDISP_WIDTH, VDISP_HEIGHT, 0, 0, 0);

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
