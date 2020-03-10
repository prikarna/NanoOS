/*
 * File    : UsbService.c
 * Remark  : Provide instant service for application installation. 
 *
 */

#include "NanoOS.h"
#include "Thread.h"
#include "Error.h"
#include "UsbData.h"
#include "UsbSerial.h"

extern BOOL					gfNeedRunApp;

static USB_DATA				sUsbInstDat;
static USB_INSTALL_RESPONSE	sResp;
static UINT16_PTR_T			spDst;
static PUSB_INSTALL_PACKET	spPack = &(sUsbInstDat.u.InstallPacket);

int UsbProgramFlash(UINT8_PTR_T pUsbRecvBuf)
{
	UINT32_T				uMax;
	BOOL					fRes = FALSE;
	UINT16_PTR_T			pSrc;
	UINT32_T				uPageNo = 0;

	NMemSet((UINT8_PTR_T) &sResp, 0, sizeof(USB_INSTALL_RESPONSE));
	sResp.Code = USB_INST_RESP__SUCCESS;

	do {

		fRes = CrIsAppRunning();
		if (fRes) {
			sResp.Code = ERR__INVALID_DEVICE_MODE;
			break;
		}

		NMemCopyFromPaddedBuffer((UINT8_PTR_T) &sUsbInstDat, pUsbRecvBuf, sizeof(USB_DATA));

		if (spPack->Index == 0xFFFF)	// Erase flash
		{
			fRes = FlsGetPageNoFromAddress((UINT32_PTR_T) APP_FLASH_WRITE_ADDRESS, &uPageNo);
			if (!fRes) {
				sResp.Code = (UINT16_T) ERR__INVALID_FLASH_PAGE_NO;
				break;
			}

			fRes = FlsUnlock();
			if (!fRes) {
				sResp.Code = (UINT16_T) ERR__FAIL_TO_UNLOCK_FLASH;
				break;
			}

			fRes = FlsErase(uPageNo, spPack->Length);
			if (!fRes) {
				sResp.Code = ERR__FAIL_TO_ERASE_FLASH;
			} else {
				sResp.Code = USB_INST_RESP__SUCCESS;
			}

			fRes = FlsLock();
		} 
		else	// Progam flash
		{
			uMax = spPack->Length / 2;

			fRes = FlsUnlock();
			if (!fRes) {
				sResp.Code = (UINT16_T) ERR__FAIL_TO_UNLOCK_FLASH;
				break;
			}

			if (spPack->Index == 0) {
				spDst = (UINT16_PTR_T) APP_FLASH_WRITE_ADDRESS;
			}

			if ((spDst >= (UINT16_PTR_T) CONFIG_WRITE_ADDRESS) ||
				((spDst + uMax) >= (UINT16_PTR_T) CONFIG_WRITE_ADDRESS))
			{
				sResp.Code = (UINT16_T) ERR__APP_SIZE_TOO_BIG;
				break;
			}

			pSrc = (UINT16_PTR_T) &(spPack->Data[0]);

			fRes = FlsProgram(spDst, pSrc, uMax);
			//DBG_PRINTF("%s: Index=%d, len.=%d, max=%d\r\n", __FUNCTION__, spPack->Index, spPack->Length, uMax);
			if (!fRes) {
				sResp.Code = ERR__FAIL_TO_PROGRAM_FLASH;
			} else {
				sResp.Code = USB_INST_RESP__SUCCESS;
				spDst += uMax;
			}

			fRes = FlsLock();
		}

	} while (FALSE);

	UsbSend((UINT8_PTR_T) &sResp, sizeof(USB_INSTALL_RESPONSE), FALSE);

	if (fRes) {
		if (spPack->Index == 0xFFFE) {
			gfNeedRunApp = TRUE;
		}
	}

	return (int) sResp.Code;
}
