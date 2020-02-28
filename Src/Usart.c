/*
 * File    : Usart.c
 * Remark  : USART service, provide access to serial TTL available in the cipset.
 *           This USART module has 2 configurations, when system is running in 
 *           low clock speed, 8000000 Hz (8 MHz), the configuration:
 *           - Baudrate     : 115200
 *           - Data bits    : 8
 *           - Parity       : None
 *           - Stop bit     : 1
 *           - Flow control : No
 *           When the system is running in high clock speed, 72000000 Hz (72 MHz),
 *           the configuration:
 *           - Baudrate     : 921600
 *           - Data bits    : 8
 *           - Parity       : None
 *           - Stop bit     : 1
 *           - Flow control : No
 *
 */

#include "Sys\CortexM\Nvic.h"
#include "Sys\CortexM\Scb.h"
#include "Sys\Stm\Rcc.h"
#include "Sys\Stm\Gpio.h"
#include "Sys\Stm\Usart.h"

#include "NanoOS.h"
#include "Thread.h"

extern UINT32_PTR_T		gpLastIntStack;

static UINT8_T			suCh;
static UINT32_T			suFault;

void UaInitialize()
{
	UINT32_T	uClk, uMant, uFract, uAHBDiv, uAPB1Div;

	uClk = ClkGetSystemClock();
	uAHBDiv = ClkGetAHBDivider();
	uAPB1Div = ClkGetAPB1Divider();

	uClk = uClk / (uAHBDiv * uAPB1Div);
	uMant = uClk / 14745600;
	
	if (uMant > 0) {
		uFract = uClk - (uMant * 14745600);
		uFract = (uFract * 16) / 14745600;
	} else {
		uMant = uClk / 1843200;

		uFract = uClk - (uMant * 1843200);
		uFract = (uFract * 16) / 1843200;
	}

	RCC_ENA_APB2_CTRL_CLK(ENABLE, RCC_APB2_CTRL__IOPB);
	RCC_ENA_APB1_CTRL_CLK(ENABLE, RCC_APB1_CTRL__USART3);
	
	GPIO_SET_MODE_OUTPUT(GPIO_PORT__B, GPIO_PIN__10, GPIO_CNF_OUTPUT__ALT_PUSH_PULL, GPIO_MODE__OUTPUT_2MHZ);
	GPIO_SET_MODE_INPUT(GPIO_PORT__B, GPIO_PIN__11, GPIO_CNF_INPUT__PULL_UP_PULL_DOWN);

	USART_ENA_RX_NOT_EMPTY_INTERRUPT(USART3, ENABLE);
	USART_ENABLE(USART3, ENABLE);
	USART_SET_BAUD_FRACT(USART3, uMant, uFract);
	USART_ENA_TX(USART3, ENABLE);
	USART_ENA_RX(USART3, ENABLE);
}

void UaWriteChar(UINT8_T ch)
{
	while (USART_IS_TX_EMPTY(USART3) == 0);
	USART_WRITE(USART3, ch);
}

__attribute__((naked))
void IntUSART3()
{
	BEGIN_EXCEPTION();

	GET_STACK_POINTER(gpLastIntStack);

	SCB_CLR_GENERAL_FAULT_STATUS();

	if (USART_IS_RX_NOT_EMPTY(USART3))
	{
		suCh = USART_READ(USART3);
		ConHandleUsartInt(suCh);
	}

	suFault = SCB_GET_GENERAL_FAULT_STATUS();
	if (suFault > 0) {
		DBG_PRINTF("%s: Fault=0x%X\r\n", __FUNCTION__, suFault);
	} else {
		gpLastIntStack = 0;
	}

	END_EXCEPTION();
}

UINT32_T UaWriteString(const char * szString)
{
	return PrnWriteString((PRINT_CHAR_TYPE) UaWriteChar, szString);
}

/*
UINT32_T UaWriteHexa(UINT8_T uIsHexCapital, UINT32_T uHex, UINT32_T uDigit)
{
	return PrnWriteHexa(UaWriteChar, uIsHexCapital, uHex, uDigit);
}
*/

UINT32_T UaPrintf(const char *szFormat, ...)
{
	UINT32_T	uRet = 0;
	va_list		argList;

	va_start(argList, szFormat);
	uRet = PrnVPrintf((PRINT_CHAR_TYPE) UaWriteChar, szFormat, argList);
	va_end(argList);

	return uRet;
}