/*
 * File    : Main.cpp
 * Remark  : Sample to demonstrate NWindow
 *
 */

#include "..\NWindow.h"
#include "Info.h"

using namespace NWindow;

/*
 * Screen sample
 *
Screen		gScreen;

void TermScreen(UINT32_T uiReason)
{
	DBG_PRINTF("Stopping application... \r\n", 0);

	Base::StopDisplayInput();
	gScreen.Destroy();
}

void OnScreenIconTouched(unsigned int uiIconId)
{
	DBG_PRINTF("%s: Icon ID = %d\r\n", __FUNCTION__, uiIconId);
}

int main(int argc, char * argv[])
{
	BOOL fRes = SetTerminationHandler(TermScreen);
	if (!fRes) {
		DBG_PRINTF("Fail to register termination handler!\r\n", 0);
		return -1;
	}

	bool b = false;
	b = gScreen.Create();
	if (!b) {
		DBG_PRINTF("Fail to create screen!\r\n", 0);
		return -3;
	}

	gScreen.OnIconTouched = OnScreenIconTouched;

	b = gScreen.Show();
	Sleep(1000);

	gScreen.IconContainer = true;

	b = Base::StartDisplayInput();
	if (!b) {
		DBG_PRINTF("Fail to start get input from display!\r\n", 0);
		gScreen.Destroy();
		return -4;
	}

	Input	Inp;
	while (true) {
		b = gScreen.GetInput(Inp);
		if (b) {
			b = gScreen.ProcessInput(Inp);
		}
	}

	return 0;
}
*/

Info gInfo;

void TermInfo(UINT32_T uiReason)
{
	DBG_PRINTF("Terminate info, reason: 0x%X\r\n", uiReason);

	Base::StopDisplayInput();
}

int main(int argc, char * argv[])
{
	bool b = false;

	BOOL fRes = SetTerminationHandler(TermInfo);
	if (!fRes) {
		DBG_PRINTF("Fail to register termination handler!\r\n", 0);
		return -1;
	}

	b = Base::StartDisplayInput();
	if (!b) {
		DBG_PRINTF("Fail to start get input from display!\r\n", 0);
		return -1;
	}

	b = gInfo.Show();

	Base::StopDisplayInput();

	return 0;
}