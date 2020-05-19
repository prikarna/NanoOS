/*
 * File    : UsbIo.cpp
 * Remark  : Main file of the UsbIo program. This program provide a USB interface between PC program (UsbIoCtrl)
 *			 and the controllers available in the chipset, currently only support GPIO and USART controllers and 
 *			 also written in C++. UsbIoCtrl can access those controller via USB (USB Serial IO). UsbIoCtrl program 
 *			 can be found in the seperate solution file, NUtils solution, with the project name of UsbIoCtrl.
 *
 */

#include "..\NanoOSApi.h"
#include "UsbIo.h"
#include "Gpio.h"
#include "Usart.h"
#include "Rcc.h"

RCC *		gpRcc;

GpioA *		gpIoA;
GpioB *		gpIoB;
GpioC *		gpIoC;
GpioD *		gpIoD;

Usart1 *	gpUa1;
Usart2 *	gpUa2;

IO_REQUEST	IoReq;
IO_RESPONSE	IoResp;

void EnableGpio(bool fEnable)
{
	switch (IoReq.Request.Gpio.Address.Port)
	{
	case GPIOPORT_A:
		gpRcc->EnableAPBClock(fEnable, false, RCC_APB_HS__GPIOA);
		break;

	case GPIOPORT_B:
		gpRcc->EnableAPBClock(fEnable, false, RCC_APB_HS__GPIOB);
		break;

	case GPIOPORT_C:
		gpRcc->EnableAPBClock(fEnable, false, RCC_APB_HS__GPIOC);
		break;

	//case GPIOPORT_D:
	//	gpRcc->EnableAPBClock(fEnable, true, RCC_APB_HS__GPIOD);
	//	break;

	default:
		break;
	}
}

bool HandleGpioReq()
{
	bool				bRes = false;
	BOOL				fRes = FALSE;

	Gpio *				pIo;
	
	switch (IoReq.Request.Gpio.Address.Port)
	{
	case GPIOPORT_A:
		pIo = dynamic_cast<Gpio *>(gpIoA);
		break;

	case GPIOPORT_B:
		pIo = dynamic_cast<Gpio *>(gpIoB);
		break;

	case GPIOPORT_C:
		pIo = dynamic_cast<Gpio *>(gpIoC);
		break;

	//case GPIOPORT_D:
	//	pIo = dynamic_cast<Gpio *>(gpIoD);
	//	break;

	default:
		IoResp.Result = IORESP_INVALID_GPIO_PARAM;
		fRes = WriteToUsbSerial((UINT8_PTR_T) &IoResp, sizeof(IO_RESPONSE));
		DBGIF_PRINTF((!fRes), "%s: WriteToUsbSerial() failed! (0x%X)\r\n", __FUNCTION__, GetLastError());
		return false;
	}

	switch (IoReq.Request.Gpio.Cmd)
	{
	case GPIOCMD_SET_AS:
		switch (IoReq.Request.Gpio.u.SetAs.Type)
		{
		case GPIOSETASTYPE_INPUT:
			EnableGpio(true);
			bRes = pIo->SetAsInput(
							static_cast<Gpio::Pin>(IoReq.Request.Gpio.Address.Pin),
							static_cast<Gpio::InputCfg>(IoReq.Request.Gpio.u.SetAs.InputConfig),
							static_cast<Gpio::PinGroup>(IoReq.Request.Gpio.Address.PinGroup)
							);
			IoResp.Result = ((bRes) ? IORESP_OK : IORESP_GPIO_OPERATION_ERROR);
			break;

		case GPIOSETASTYPE_OUPUT:
			EnableGpio(true);
			bRes = pIo->SetAsOutput(
							static_cast<Gpio::Pin>(IoReq.Request.Gpio.Address.Pin),
							static_cast<Gpio::OutputCfg>(IoReq.Request.Gpio.u.SetAs.OutputConfig),
							static_cast<Gpio::OutputMode>(IoReq.Request.Gpio.u.SetAs.OutputMode),
							static_cast<Gpio::PinGroup>(IoReq.Request.Gpio.Address.PinGroup)
							);
			IoResp.Result = ((bRes) ? IORESP_OK : IORESP_GPIO_OPERATION_ERROR);
			break;

		default:
			IoResp.Result = IORESP_INVALID_GPIO_PARAM;
			break;
		}
		break;


	case GPIOCMD_WRITE:
		bRes = pIo->Write(
					static_cast<Gpio::Pin>(IoReq.Request.Gpio.Address.Pin),
					static_cast<Gpio::PinGroup>(IoReq.Request.Gpio.Address.PinGroup),
					IoReq.Request.Gpio.u.Write.Data
					);
		IoResp.Result = ((bRes) ? IORESP_OK : IORESP_GPIO_OPERATION_ERROR);
		break;

	case GPIOCMD_READ:
		bRes = pIo->Read(
					static_cast<Gpio::Pin>(IoReq.Request.Gpio.Address.Pin),
					static_cast<Gpio::PinGroup>(IoReq.Request.Gpio.Address.PinGroup), 
					IoResp.Data[0]
					);
		if (bRes) {
			IoResp.Length = 1;
			IoResp.Result = IORESP_OK;
		} else {
			IoResp.Result = IORESP_GPIO_OPERATION_ERROR;
		}
		break;

	default:
		IoResp.Result = IORESP_INVALID_GPIO_CMD;
		break;
	}

	fRes = WriteToUsbSerial((UINT8_PTR_T) &IoResp, sizeof(IO_RESPONSE));
	DBGIF_PRINTF((!fRes), "%s: WriteToUsbSerial() failed! (0x%X)\r\n", __FUNCTION__, GetLastError());

	return bRes;
}

bool HandleUsartReq()
{
	BOOL	fRes = FALSE;
	Usart *	pUa;

	switch (IoReq.Request.Usart.Address)
	{
	case USARTADDR_1:
		pUa = dynamic_cast<Usart *>(gpUa1);
		break;

	case USARTADDR_2:
		pUa = dynamic_cast<Usart *>(gpUa2);
		break;

	default:
		IoResp.Result = IORESP_INVALID_USART_PARAM;
		fRes = WriteToUsbSerial((UINT8_PTR_T) &IoResp, sizeof(IO_RESPONSE));
		DBGIF_PRINTF((!fRes), "%s: WriteToUsbSerial() failed! (0x%X)\r\n", __FUNCTION__, GetLastError());
		return false;
	}

	bool			bRes = false;
	unsigned char *	pDat;
	unsigned char	uCount;

	switch (IoReq.Request.Usart.Cmd)
	{
	case USARTCMD_OPEN:
		bRes = pUa->Open(
					static_cast<Usart::BaudRate>(IoReq.Request.Usart.u.Open.BaudRate),
					static_cast<Usart::DataBits>(IoReq.Request.Usart.u.Open.DataBits),
					static_cast<Usart::Parity>(IoReq.Request.Usart.u.Open.Parity),
					static_cast<Usart::StopBit>(IoReq.Request.Usart.u.Open.StopBit),
					static_cast<Usart::FlowControl>(IoReq.Request.Usart.u.Open.FlowControl)
					);
		IoResp.Result = ((bRes) ? IORESP_OK : IORESP_USART_OPERATION_ERROR);
		break;

	case USARTCMD_WRITE:
		if (IoReq.Request.Usart.u.Write.Length > MAX_IO_DATA) {
			IoResp.Result = IORESP_INVALID_USART_PARAM;
		} else {
			pDat = &(IoReq.Request.Usart.u.Write.Data[0]);
			for (uCount = 0; uCount < IoReq.Request.Usart.u.Write.Length; uCount++)
			{
				bRes = pUa->Write(*pDat);
				if (!bRes) break;
				pDat++;
			}
			if (bRes) {
				IoResp.Length = uCount;
				IoResp.Result = IORESP_OK;
			} else {
				IoResp.Result = IORESP_USART_OPERATION_ERROR;
			}
		}
		break;

	case USARTCMD_READ:
		if (IoReq.Request.Usart.u.Read.Length > MAX_IO_DATA) {
			IoResp.Result = IORESP_INVALID_USART_PARAM;
		} else {
			for (uCount = 0; uCount < IoReq.Request.Usart.u.Read.Length; uCount++)
			{
				bRes = pUa->Read(IoResp.Data[uCount]);
				if (!bRes) break;
			}
			if (bRes) {
				IoResp.Length = uCount;
				IoResp.Result = IORESP_OK;
			} else {
				IoResp.Result = IORESP_USART_OPERATION_ERROR;
			}
		}
		break;

	case USARTCMD_CLOSE:
		bRes = pUa->Close();
		IoResp.Result = ((bRes) ? IORESP_OK : IORESP_USART_OPERATION_ERROR);
		break;

	default:
		IoResp.Result = IORESP_INVALID_USART_CMD;
		break;
	}

	fRes = WriteToUsbSerial((UINT8_PTR_T) &IoResp, sizeof(IO_RESPONSE));
	DBGIF_PRINTF((!fRes), "%s: WriteToUsbSerial() failed! (0x%X)\r\n", __FUNCTION__, GetLastError());

	return bRes;
}

bool HandleIoRequest()
{
	BOOL	fRes = FALSE;
	bool	bRes = false;

	MemSet((UINT8_PTR_T) &IoResp, 0, sizeof(IO_RESPONSE));

	switch (IoReq.Type)
	{
	case IOTYPE_GPIO:
		bRes = HandleGpioReq();
		break;

	case IOTYPE_USART:
		bRes = HandleUsartReq();
		break;

	default:
		IoResp.Result = IORESP_UNKNOWN_IO_REQ;
		fRes = WriteToUsbSerial((UINT8_PTR_T) &IoResp, sizeof(IO_RESPONSE));
		DBGIF_PRINTF((!fRes), "%s: WriteToUsbSerial() failed! (0x%X)\r\n", __FUNCTION__, GetLastError());
		break;
	}

	return bRes;
}

int main(int , char *[])
{
	gpRcc	= new RCC();

	/*
	 * Initialize IOs
	 */
	gpIoA	= new GpioA();
	gpIoB	= new GpioB();
	gpIoC	= new GpioC();
	gpIoD	= new GpioD();

	gpUa1	= new Usart1();
	gpUa2	= new Usart2();

	BOOL		fRes = FALSE;
	UINT32_T	uiRead;
	UINT32_T	uiErr;

	while (true) {
		MemSet((UINT8_PTR_T) &IoReq, 0, sizeof(IO_REQUEST));
		uiRead = 0;
		fRes = ReadFromUsbSerial((UINT8_PTR_T) &IoReq, sizeof(IO_REQUEST), &uiRead);
		if (!fRes) {
			uiErr = GetLastError();
			if (uiErr == ERR__USB_IO_IS_NOT_READY) {
				DBG_PRINTF("%s: USB is not ready, wait a while...\r\n", __FUNCTION__);
				Sleep(600);
			} else if (uiErr == ERR__USB_IO_IS_BUSY) {
				DBG_PRINTF("%s: USB is busy, cancel and wait a while...\r\n", __FUNCTION__);
				CancelReadFromUsbSerial();
				Sleep(600);
			} else {
				DBG_PRINTF("%s: ReadFromUsbSerial() failed (0x%X)!\r\n", __FUNCTION__, uiErr);
			}

			continue;
		}

		if (uiRead != sizeof(IO_REQUEST)) {
			DBG_PRINTF("%s: Warning: IO request data corruption!\r\n", __FUNCTION__);
			continue;
		}

		HandleIoRequest();	
	}

	return 0;
}