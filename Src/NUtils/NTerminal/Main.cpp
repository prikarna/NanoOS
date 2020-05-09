/*
 * File    : Main.cpp
 * Remark  : Main or program startup.
 *
 */

#include "NWind.h"

NWind	gWind;

int __stdcall _tWinMain(HINSTANCE hInst, HINSTANCE , LPTSTR , int iCmdShow)
{
	if (!gWind.Create(hInst))
		return -1;

	return gWind.Show();
}
