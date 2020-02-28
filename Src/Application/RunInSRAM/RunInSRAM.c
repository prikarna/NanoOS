/*
 * File    : RunInSRAM.c
 * Remark  : Sample to demonstrate program that can be ran in SRAM. This is the same as any other samples except
 *           for the linker script. The linker script put .text section in SRAM.
 *           
 *           *** Note:
 *           NanoOS can not guaranty that run in SRAM is faster than run in FLASH.
 *
 */

#include "..\NanoOSApi.h"

#include "..\..\Sys\Stm\Gpio.h"
#include "..\..\Sys\Stm\Rcc.h"

#define DELAY_COUNT			1000000
#define GET_PC(Var)			__asm volatile ("MOV %0, PC" : "=r" (Var))

int			giCount = 0;
int			giVar = 100;
const int	gciConst = 0xADEADBEE;

int FlipLED(int iMax)
{
	UINT32_T	u;
	UINT32_T	uCount;
	UINT32_T	uMax = 10;

	if (iMax > 0) uMax = iMax;

	RCC_ENA_APB2_CTRL_CLK(ENABLE, RCC_APB2_CTRL__IOPC);

	GPIO_SET_MODE_OUTPUT(GPIO_PORT__C, GPIO_PIN__13, GPIO_CNF_OUTPUT__GEN_PUSH_PULL, GPIO_MODE__OUTPUT_2MHZ);

	for (u = 0; u < uMax; u++) {
		GPIO_WRITE(GPIO_PORT__C, GPIO_PIN__13, LOW);
		uCount = (DELAY_COUNT / 2);
		while (uCount-- > 0);

		GPIO_WRITE(GPIO_PORT__C, GPIO_PIN__13, HIGH);
		uCount = (DELAY_COUNT / 2);
		while (uCount-- > 0);

	}

	return (int) u;
}

int main(int argc, char * argv[])
{
	UINT32_T	uCount;
	UINT32_T	uPC = 0;

	/*
	 * Get current PC (Program Counter) register. If PC is in range of FLASH memory then this application run in FLASH 
	 * memory. If PC is in range of SRAM then this application run in SRAM. FLASH and SRAM start addresses and their 
	 * range can be seen in ld script, in this case: RunInSRAM.ld
	 */
	GET_PC(uPC);

	Printf("\r\nRun in flash or sram. (PC=0x%X)\r\n", uPC);
	Printf("giVar = %d (initialized global variable), gciConst = 0x%X (global constant).\r\n", giVar, gciConst);
	Printf("Print \"test\" 5 times with counted delay=%d\r\n", DELAY_COUNT);

	for (giCount = 1; giCount <= 5; giCount++) 
	{
		uCount = DELAY_COUNT;
		while (uCount-- > 0);
		Printf("[%d] test\r\n", giCount);
	}

	Printf("Run test LED.\r\n");
	FlipLED(20);

	Printf("Program end.\r\n");

	return 0;
}
