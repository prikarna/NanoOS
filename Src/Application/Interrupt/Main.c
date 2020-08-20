/*
 * File    : Main.c
 * Remark  : Simple sample to demonstrate how to use interrupt in NanoOS.
 *           Ensure -R<path_to_NanoOS_elf> option for linker object be specified
 *           in the Makefile, because this will use NanoOS functions directly.
 *           In this program we generate interrupt via EXTI controller which available
 *           on the chipset and use event object to synchronize execution between 
 *           thread and interrupt handler. 
 *
 */

#include "..\..\NanoOS.h"

#ifdef DBG_PRINTF
# undef DBG_PRINTF
#endif

#include "..\..\Sys\Stm\ExtInt.h"
#include "..\NanoOSApi.h"

UINT32_T gEvtId = 0;
UINT32_T guiPendInt = 0;

/*
 * Interrupt handler.
 * This function is called in interrupt level, so we can't use NanoOS API. Note that almost
 * all functions specified in NanoOSApi.h doesn't not support to be called from interrupt
 * level. So we use NanoOS function in NanoOS.h directly. In this case we use EvtSet() 
 * rather than SetEvent().
 */
void HandleSoftInt(UINT32_T uiIntType)
{
	UaPrintf("%s: Int. type=0x%2X\r\n", __FUNCTION__, uiIntType);	// Output to console

	if ((uiIntType == INT_TYPE__EXTI0) ||
		(uiIntType == INT_TYPE__EXTI1) ||
		(uiIntType == INT_TYPE__EXTI2) ||
		(uiIntType == INT_TYPE__EXTI3))
	{
		guiPendInt = EXTI_GET_PEND_INT();
		EXTI_CLR_PEND_INT(guiPendInt);	// Must clear this pending interrupt.

		EvtSet(gEvtId, 0);
	}
	else
	{
		UaPrintf("%s: Unexpected interrupt 0x%2X\r\n", __FUNCTION__, uiIntType);
	}
}

int main(int argc, char * argv[])
{
	BOOL		fRes = FALSE;

	Printf("Software interrupt demo.\r\n");

	fRes = SetInterruptHandler(HandleSoftInt);
	if (!fRes) {
		Printf("Fail to set interrupt handler! (0x%X)\r\n", GetLastError());
		return -2;
	}

	fRes = CreateEvent(&gEvtId);
	if (!fRes) {
		Printf("Fail to create event! (0x%X)\r\n", GetLastError());
		SetInterruptHandler(0);
		return -2;
	}

	UINT32_T uiExtLines = (EXTI_LINE_0 | 
						   EXTI_LINE_1 | 
						   EXTI_LINE_2 | 
						   EXTI_LINE_3); 
	UINT32_T uiLineTrig;

	// Enable 4 external line interrupts, line 0~3
	EXTI_ENA_INTR(TRUE, uiExtLines);

	for (UINT8_T u = 0; u < 4; u++) {
		uiLineTrig = ((UINT32_T) 0x1) << u;
		Printf("Trigger line %d interrupt...\r\n", u);

		EXTI_TRIG_INT(uiLineTrig);	// Trigger line 0~3 one by one via software

		fRes = WaitForObject(THREAD_WAIT_OBJ__EVENT, gEvtId, THREAD__INFINITE_WAIT);
		if (!fRes) {
			Printf("Fail to wait for object! (0x%X)\r\n", GetLastError());
			break;
		}
		Printf("Cleared pending int. = %d\r\n", guiPendInt);
		ResetEvent(gEvtId);
	}

	EXTI_ENA_INTR(FALSE, uiExtLines);

	CloseEvent(gEvtId);

	return 0;
}
