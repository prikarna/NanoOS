/*
 * File    : Main.cpp
 * Remark  : Program entry point for NanoOS Port Service.
 *
 */

#include "NService.h"
#include "NPortSvc.h"

#include "..\NTerminal\Debug.h"

int _tmain(int argc, TCHAR * szArgv[])
{
	NServiceDispatcher	SvcDisp;
	NPortSvc			Svc;

	bool	bRes = false;
	bRes = SvcDisp.Run(&Svc);

	return ((bRes) ? 0 : -1);
}