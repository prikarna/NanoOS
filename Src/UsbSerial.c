/*
 * File    : UsbSerial.c
 * Remark  : USB Serial converter handlers. These routines 'turn'
 *           USB interface to COM or Serial port like. Application 
 *           in PC can treat this as serial port or COM port.
 *
 */

#include "Sys\Stm\Usb.h"
#include "Sys\Stm\Gpio.h"
#include "Sys\Stm\Rcc.h"

#include "NanoOS.h"
#include "Thread.h"
#include "Service.h"
#include "Error.h"
#include "UsbSerial.h"
#include "UsbData.h"

extern const UINT8_T	gDevDesc[];
extern const UINT8_T	gCfgDesc[];
extern const UINT8_T	gLangId[];
extern const UINT8_T	gMfcStr[];
extern const UINT8_T	gProdStr[];
extern const UINT8_T	gSerStr[];

BOOL					gfNeedRunApp;

static PUSB_BUFFER		spUsbBuff;
static LINE_CODING		sLineCoding;
static USB_SETUP		sStp;
static UINT8_PTR_T		spEpBuf;
static UINT16_T			sControlLineState;

/*
 * Variables for main int handler
 */
static UINT32_T			suInt;
static UINT8_T			suIsStpReq;
static UINT8_T			suDir;
static UINT8_T			suEpReg;

static UINT8_PTR_T		spTxBuf;
static UINT8_PTR_T		spRxBuf;

static UINT8_T		suRecvFlags;

static PCONFIG		spCfg;

void UsbShutdown()
{
	UINT32_T	u;

	USB_SET_CTL(SET, USB_CTL__FORCE_SUSPEND);
	USB_CLR_ALL_INTS();
	USB_SET_CTL(SET, USB_CTL__POWER_DOWN);

	UsbCancelReceive();

	u = 8;
	while (u-- > 0);	// Simulate approx. 1 micro sec. delay (maybe far from acurrate)

	RCC_ENA_APB1_CTRL_CLK(DISABLE, RCC_APB1_CTRL__USB);

	GPIO_SET_MODE_OUTPUT(GPIO_PORT__A, GPIO_PIN__11, GPIO_CNF_OUTPUT__GEN_PUSH_PULL, GPIO_MODE__OUTPUT_50MHZ);
	GPIO_SET_MODE_OUTPUT(GPIO_PORT__A, GPIO_PIN__12, GPIO_CNF_OUTPUT__GEN_PUSH_PULL, GPIO_MODE__OUTPUT_50MHZ);
	GPIO_WRITE(GPIO_PORT__A, GPIO_PIN__11, LOW);
	GPIO_WRITE(GPIO_PORT__A, GPIO_PIN__12, LOW);

	UaWriteString("Shutting down USB, please wait... ");
	u = 2400000;
	while (u-- > 0);
	UaWriteString("done.\r\n");
}

void UsbInitialize()
{
	UINT32_T		u;

	RCC_ENA_APB2_CTRL_CLK(ENABLE, RCC_APB2_CTRL__IOPA);

	gfNeedRunApp = FALSE;
	spCfg = CfgGet();

	spEpBuf = 0;
	sControlLineState = 0;

	suRecvFlags = USB_RECV__NONE;

	/*
	 * Serial port info
	 */
	sLineCoding.bCharFormat		= 0;
	sLineCoding.bDataBits		= 8;
	sLineCoding.bParityType		= 0;
	sLineCoding.dwDTERate		= 921600;

	/*
	 * Initialize USB
	 */
	RCC_SET_USB_PRESC(RCC_USB_PRESCALE__DIV_1_DOT_5);
	RCC_ENA_APB1_CTRL_CLK(ENABLE, RCC_APB1_CTRL__USB);

	/*
	 * Initialize USB buffer with USB_BTABLE = 0
	 */
	NMemSet((UINT8_PTR_T) USB_BUFF_ADDR, 0, sizeof(USB_BUFFER));
	spUsbBuff					= (PUSB_BUFFER)(USB_BUFF_ADDR);

	u = SIZEOF_USB_BUFFER;
	spUsbBuff->Ep0TxOffset		= (UINT16_T) u;
	spUsbBuff->Ep0TxLength		= 0;

	u += USBEP0_TX_BUF_LEN;
	spUsbBuff->Ep0RxOffset		= (UINT16_T) u;
	spUsbBuff->Ep0RxLength		= USB_RX_BUFFER_LENGTH(1, 1);	// 64 bytes

	u += USBEP0_RX_BUF_LEN;
	spUsbBuff->Ep1TxOffset		= (UINT16_T) u;
	spUsbBuff->Ep1TxLength		= 0;

	u += USBEP1_BUF_LEN;
	spUsbBuff->Ep2TxOffset		= (UINT16_T) u;
	spUsbBuff->Ep2TxLength		= 0;

	u += (USBEP2_BUF_LEN + USBEP2_RSVD_BUF_LEN);
	spUsbBuff->Ep3RxOffset		= (UINT16_T) u;
	spUsbBuff->Ep3RxLength		= USB_RX_BUFFER_LENGTH(1, 1);	// 64 bytes

	spTxBuf = USB_GET_BUFFER_ADDRESS(spUsbBuff->Ep2TxOffset);
	spRxBuf = USB_GET_BUFFER_ADDRESS(spUsbBuff->Ep3RxOffset);

	USB_SET_CTL(SET, (USB_CTL__EN_CORECT_TRANS_INT |			\
					  USB_CTL__EN_ERROR_INT |					\
					  USB_CTL__EN_RESET_INT |					\
					  USB_CTL__EN_SUSPEND_INT |					\
					  USB_CTL__EN_WAKE_UP_INT));

	USB_SET_CTL(CLEAR, USB_CTL__POWER_DOWN);

	u = 8;
	while (u-- > 0);	// Simulate approx. 1 micro sec. delay (far from accurate)

	USB_CLR_ALL_INTS();
	USB_SET_CTL(CLEAR, USB_CTL__FORCE_USB_RESET);
}

static void HandleSetupDefault()
{
	UINT8_T		uDir = USB_GET_TRANSACT_DIR();

	if (uDir == 1) {
		spUsbBuff->Ep0TxLength = 0;
		USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN_STALL);
	} else {
		USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_OUT_STALL);
	}
}

static void GetStatus()
{
	UINT8_T		uDest = (UINT8_T) (sStp.bmRequestType & 0xF);
	UINT16_T	uStat = 0;

	switch (uDest)
	{
	case 0:
		uStat = 0x2;
		break;

	case 1:
		uStat = 0x0;
		break;

	case 2:
		uStat = 0x0;
		break;

	default:
		HandleSetupDefault();
		return;
	}

	spEpBuf = USB_GET_BUFFER_ADDRESS(spUsbBuff->Ep0TxOffset);
	NMemCopyToPaddedBuffer(spEpBuf, (UINT8_PTR_T) &uStat, 2);

	USB_EP_SET_TX_BUF_LEN(USB_EP_REG_0, 0, 2);
	USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN);
}

static void GetDeviceDescriptor()
{
	spEpBuf = USB_GET_BUFFER_ADDRESS(spUsbBuff->Ep0TxOffset);
	NMemCopyToPaddedBuffer(spEpBuf, (UINT8_PTR_T) &gDevDesc[0], sizeof(USB_DEVICE_DESCRIPTOR));

	spUsbBuff->Ep0TxLength = sStp.wLength;
	USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN);
}

static void GetConfigurationDescriptor()
{
	UINT32_T						uLen;
	PUSB_CONFIGURATION_DESCRIPTOR	pCfg;

	if (sStp.wLength <= (UINT16_T) USBEP0_TX_BUF_LEN)
	{
		uLen = sStp.wLength;
	}
	else if (sStp.wLength >= 0xFF)
	{
		pCfg = (PUSB_CONFIGURATION_DESCRIPTOR) &gCfgDesc[0];
		uLen = pCfg->wTotalLength;
	}
	else
	{
		HandleSetupDefault();
		return;
	}

	spEpBuf = USB_GET_BUFFER_ADDRESS(spUsbBuff->Ep0TxOffset);
	NMemCopyToPaddedBuffer(spEpBuf, (UINT8_PTR_T) &gCfgDesc[0], uLen);

	spUsbBuff->Ep0TxLength = uLen;
	USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN);
}

static void GetStringDescriptor()
{
	UINT8_PTR_T	pStr = 0;
	UINT8_T		uStrIdx = sStp.wValue & 0xFF;
	UINT32_T	uLen = 0;

	switch (uStrIdx)
	{
	case 0:
		pStr = (UINT8_PTR_T) &gLangId[0];
		uLen = gLangId[0];
		break;

	case 1:
		pStr = (UINT8_PTR_T) &gMfcStr[0];
		uLen = gMfcStr[0];
		break;

	case 2:
		pStr = (UINT8_PTR_T) &gProdStr[0];
		uLen = gProdStr[0];
		break;

	case 3:
		pStr = (UINT8_PTR_T) &gSerStr[0];
		uLen = gSerStr[0];
		break;

	default:
		HandleSetupDefault();
		return;
	}

	spEpBuf = USB_GET_BUFFER_ADDRESS(spUsbBuff->Ep0TxOffset);
	NMemCopyToPaddedBuffer(spEpBuf, pStr, uLen);

	spUsbBuff->Ep0TxLength = uLen;
	USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN);
}

static void GetLineCoding()
{
	spEpBuf = USB_GET_BUFFER_ADDRESS(spUsbBuff->Ep0TxOffset);
	
	NMemCopyToPaddedBuffer(spEpBuf, (UINT8_PTR_T) &sLineCoding, sStp.wLength);

	spUsbBuff->Ep0TxLength = sStp.wLength;
	USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN);
}

static void HandleSetupInt()
{
	UINT16_T		uReq;
	UINT8_T			uDescType;

	spEpBuf = USB_GET_BUFFER_ADDRESS(spUsbBuff->Ep0RxOffset);
	NMemCopyFromPaddedBuffer((UINT8_PTR_T) &sStp, spEpBuf, sizeof(USB_SETUP));

	uReq = ((((UINT16_T) sStp.bmRequestType) << 8) | sStp.bRequest);

	switch (uReq)
	{
	case USBSTD__GET_STATUS:
		GetStatus();
		break;

	case USBSTD__GET_DESCRIPTOR:
	case USBSTD__GET_DESCRIPTOR_INTERFACE:
		uDescType = (UINT8_T) ((sStp.wValue >> 8) & 0xFF);
		switch (uDescType)
		{
		case DESCTYPE__DEVICE:
			GetDeviceDescriptor();
			break;

		case DESCTYPE__CONFIGURATION:
			GetConfigurationDescriptor();
			break;

		case DESCTYPE__DEVICE_QUALIFIER:
			HandleSetupDefault();
			break;

		case DESCTYPE__STRING:
			GetStringDescriptor();
			break;

		default:
			DBG_PRINTF("%s: Unknown USB descriptor request (bmRequestType=0x%2X, bRequest=0x%2X)\r\n", 
				sStp.bmRequestType, sStp.bRequest);
			HandleSetupDefault();
			break;
		}
		break;

	case USBSTD__SET_ADDRESS:
	case USBSTD__SET_CONFIGURATION:
		spUsbBuff->Ep0TxLength = 0;
		USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN);
		break;

	case USBCDC__GET_LINE_CODING:
		GetLineCoding();
		break;

	case USBCDC__SET_LINE_CODING:
		USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_OUT);
		break;

	case USBCDC__SET_CONTROL_LINE_STATE:
		spUsbBuff->Ep0TxLength = 0;
		USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN);
		break;

	case USBSTD__SET_DESCRIPTOR:
		HandleSetupDefault();
		break;

	default:
		DBG_PRINTF("%s: Unknown USB setup request type (bmRequestType=0x%2X, bRequest=0x%2X)\r\n", 
			__FUNCTION__, sStp.bmRequestType, sStp.bRequest);
		HandleSetupDefault();
		break;
	}
}

static void SetLineCoding(UINT8_T uDir)
{
	if (uDir == 0) {
		USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN_END);
	} else {

		spEpBuf = USB_GET_BUFFER_ADDRESS(spUsbBuff->Ep0RxOffset);

		NMemCopyFromPaddedBuffer((UINT8_PTR_T) &sLineCoding, spEpBuf, sizeof(LINE_CODING));

		spUsbBuff->Ep0TxLength = 0;
		USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_OUT_TO_IN);
	}
}

static void HandleEpReg0Int(UINT8_T uDir)
{
	UINT16_T		uReq;
	UINT8_T			uDescType;

	uReq = ((((UINT16_T) sStp.bmRequestType) << 8) | sStp.bRequest);

	switch (uReq)
	{
	case USBSTD__GET_STATUS:
		if (uDir == 0) {
			USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN_TO_OUT);
		} else {
			USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_OUT_END);
		}
		break;

	case USBSTD__GET_DESCRIPTOR:
	case USBSTD__GET_DESCRIPTOR_INTERFACE:
		uDescType = (UINT8_T) ((sStp.wValue >> 8) & 0xFF);
		switch (uDescType)
		{
		case DESCTYPE__DEVICE:
		case DESCTYPE__CONFIGURATION:
		case DESCTYPE__STRING:
			if (uDir == 0) {
				USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN_TO_OUT);
			} else {
				USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_OUT_END);
			}
			break;

		default:
			TRAP_EXEC(1, TRAP__UNKNOWN_USB_DESCRIPTOR_TYPE, sStp.bmRequestType, sStp.bRequest);
			break;
		}
		break;

	case USBSTD__SET_ADDRESS:
		USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN_END);
		USB_SET_DEV_ADDR(sStp.wValue);
		break;

	case USBSTD__SET_CONFIGURATION:
		USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN_END);
		break;

	case USBCDC__GET_LINE_CODING:
		if (uDir == 0) {
			USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN_TO_OUT);
		} else {
			USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_OUT_END);
		}
		break;

	case USBCDC__SET_LINE_CODING:
		SetLineCoding(uDir);
		break;


	case USBCDC__SET_CONTROL_LINE_STATE:
		USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN_END);
		sControlLineState = sStp.wValue;
		break;


	case USBSTD__SET_DESCRIPTOR:
		break;

	default:
		TRAP_EXEC(1, TRAP__UNKNOWN_USB_DESCRIPTOR_TYPE, sStp.bmRequestType, sStp.bRequest);
		break;
	}
}

static void HandleEpReg1Int()
{
	USB_EP_CTL_EX(USB_EP_REG_1, 1, USB_EP_CTL_EX__INTERRUPT_IN_CLEAR);
}

static void HandleEpReg2Int()
{
	USB_EP_CTL_EX(USB_EP_REG_2, 2, USB_EP_CTL_EX__BULK_IN_CLEAR);

	if (gfNeedRunApp) {
		gfNeedRunApp = FALSE;
		if (spCfg->AutoRun == FALSE) {
			return;
		}
		ThdCreate(
				0,
				spCfg->Control,
				FALSE,
				(THREAD_ENTRY_TYPE) APP_FLASH_RUN_ADDRESS,
				0,
				0
				);
	}
}

static void HandleEpReg3Int()
{
	UINT16_PTR_T	pInstSig;
	
	pInstSig = (UINT16_PTR_T) spRxBuf;
	if (*pInstSig == USB_DATA_TYPE__INSTALL) {
		UsbProgramFlash(spRxBuf);
	} else {
		UsbCompleteReceive();
	}
	
	USB_EP_CTL_EX(USB_EP_REG_3, 3, USB_EP_CTL_EX__BULK_OUT_CLEAR_VALID);
}

void IntUSBLowPriorityOrCAN1_RX0()
{
	suInt = USB_GET_ALL_INTS();

	if (suInt & USB_INT__CORRECT_TRANS)
	{
		suIsStpReq = USB_EP_IS_SETUP(USB_EP_REG_0);
		if (suIsStpReq) {
			HandleSetupInt();
		} else {

			suEpReg = USB_GET_TRANSACT_EP_ID();

			if (suEpReg == USB_EP_REG_0) {

				suDir = USB_GET_TRANSACT_DIR();
				HandleEpReg0Int(suDir);
			
			} else if (suEpReg == USB_EP_REG_1) {

				HandleEpReg1Int();
			
			} else if (suEpReg == USB_EP_REG_2) {

				HandleEpReg2Int();
			
			} else if (suEpReg == USB_EP_REG_3) {

				HandleEpReg3Int();
			
			} else {
				TRAP_EXEC(1, TRAP__UNEXPECTED_USB_ENDPOINT_REGISTER, suEpReg, 0);
			}
		}
	}
	else if (suInt & USB_INT__RESET)
	{
		USB_SET_BTBL_ADDR(0);
		USB_SET_DEV_ADDR(0);
		USB_ENABLE(ENABLE);

		/*
		 *	Endpoint 0 (control-IN-OUT endpoint) buffer
		 */
		USB_EP_SET(USB_EP_REG_0, USB_EP_TYPE__CONTROL, USB_EP_KIND__NONE, 0);
		USB_EP_CTL(USB_EP_REG_0, USB_EP_CTL__CONTROL_IN_AND_OUT_INIT);		/* Endpoint 0 (control-IN-OUT) init */

		/*
		 *	Endpoint 1 (interrupt-IN endpoint) buffer
		 */
		USB_EP_SET(USB_EP_REG_1, USB_EP_TYPE__INTERRUPT, USB_EP_KIND__NONE, 1);
		USB_EP_CTL_EX(USB_EP_REG_1, 1, USB_EP_CTL_EX__INTERRUPT_IN_INIT);	/* Endpoint 1 (interrupt-IN) init */

		/*
		 *	Endpoint 2 (bulk-IN endpoint) buffer
		 */
		USB_EP_SET(USB_EP_REG_2, USB_EP_TYPE__BULK, USB_EP_KIND__NONE, 2);
		USB_EP_CTL_EX(USB_EP_REG_2, 2, USB_EP_CTL_EX__BULK_IN_INIT);			/* Endpoint 2 (bulk-IN) init */

		/*
		 *	Endpoint 3 (bulk-OUT endpoint) buffer
		 */
		USB_EP_SET(USB_EP_REG_3, USB_EP_TYPE__BULK, USB_EP_KIND__NONE, 3);
		USB_EP_CTL_EX(USB_EP_REG_3, 3, USB_EP_CTL_EX__BULK_OUT_INIT);		/* Endpoint 3 (bulk-OUT) init */

		USB_CLR_RESET_INT();

		suRecvFlags = USB_RECV__NONE;
	}
	else if (suInt & USB_INT__SUSPEND)
	{
		USB_SET_FORCE_SUSPEND(SET);
		USB_CLR_SUSPEND_INT();

		suRecvFlags = USB_RECV__NONE;
	}
	else if (suInt & USB_INT__WAKEUP)
	{
		USB_SET_FORCE_SUSPEND(CLEAR);
		USB_CLR_WAKE_UP_INT();
	}
	else if (suInt & USB_INT__ERROR)
	{
		DBG_PRINTF("%s: ERROR!!!\r\n", __FUNCTION__);
		USB_CLR_ERR_INT();

		suRecvFlags = USB_RECV__NONE;
	}
	else if (suInt > 0)
	{
		USB_CLEAR_UNUSED_INTRS();
	}
	else
	{
		if (suInt > 0) {
			TRAP_EXEC(1, TRAP__UNKNOWN_USB_INTERRUPT, USB_GET_ALL_INTS(), 0);
		}
	}
}

UINT8_T UsbSend(UINT8_PTR_T pBuffer, UINT32_T uiBufferByteLength, BOOL fWait)
{
	UINT32_T		uLen;
	UINT32_T		ui;
	BOOL			fRes = FALSE;

	if (sControlLineState != 0x3) {
		ThdSetLastError(ERR__USB_IO_IS_NOT_READY);
		return FALSE;
	}

	if ((pBuffer == 0) || (uiBufferByteLength == 0)) {
		ThdSetLastError(ERR__INVALID_PARAMETER);
		return 0;
	}

	uLen = ((uiBufferByteLength <= USB_DATA_SIZE) ? uiBufferByteLength : USB_DATA_SIZE);

	NMemCopyToPaddedBuffer(spTxBuf, pBuffer, uLen);
	spUsbBuff->Ep2TxLength = uLen;

	USB_EP_CTL_EX(USB_EP_REG_2, 2, USB_EP_CTL_EX__BULK_IN_VALID);

	if (!fWait) {
		return TRUE;
	}

	while (TRUE) {
		ui = USB_GET_ALL_INTS();
		if (ui & USB_IO_INT_ERRORS) {
			ThdSetLastError(ERR__USB_IO);
			break;
		}

		if (USB_EP_GET_TX_CTR(USB_EP_REG_2) == 1) {
			fRes = TRUE;
			break;
		}

		ui = RCC_GET_APB1_CTRL_CLK();
		if ((ui & RCC_APB1_CTRL__USB) == 0) {
			ThdSetLastError(ERR__USB_IO);
			break;
		}
	}

	return fRes;
}

BOOL UsbRequestReceive()
{
	if (sControlLineState != 0x3) {
		ThdSetLastError(ERR__USB_IO_IS_NOT_READY);
		return FALSE;
	}

	if (suRecvFlags != USB_RECV__NONE) {
		ThdSetLastError(ERR__USB_IO_IS_BUSY);
		return FALSE;
	}

	suRecvFlags = USB_RECV__IN_PROGRESS;
	return TRUE;
}

BOOL UsbCompleteReceive()
{
	UINT32_PTR_T	pStack;
	PSVC_PARAM		pSvcParms;
	PTHREAD			pTh;
	UINT32_T		uLen = 0;
	UINT32_T		uRecvLen = 0;
	PINITIAL_THREAD_STACK	pInitTh;

	pTh = ThdGetWaitingInterrupt(0, THREAD_FLAG__IO_RECEIVE);
	if (!pTh) {
		return FALSE;
	}

	pTh->Flags &= ~(THREAD_FLAG__IO_RECEIVE);

	pStack = pTh->LastStackPtr;
	pStack += 2;
	pSvcParms = (PSVC_PARAM) pStack;
	
	pInitTh = (PINITIAL_THREAD_STACK) pTh->LastStackPtr;
	pInitTh->ExcFrame.PC = pTh->SleepPC;

	/*
	 * Remark to save space
	 *
	if (suRecvFlags != USB_RECV__IN_PROGRESS) {

		pTh->LastError = ERR__USB_IO_STATE;
		pSvcParms->ReturnValue = FALSE;
	
	} else {

		uRecvLen = USB_GET_RX_BUFFER_COUNT(spUsbBuff->Ep3RxLength);
		uLen = (((UINT32_T) pSvcParms->Params[1]) > uRecvLen) ? uRecvLen : ((UINT32_T) pSvcParms->Params[1]);

		NMemCopyFromPaddedBuffer((UINT8_PTR_T) pSvcParms->Params[0], spRxBuf, uLen);
		if (pSvcParms->Params[2] != 0) {
			*((UINT32_PTR_T) pSvcParms->Params[2]) = uLen;
		}

		suRecvFlags = USB_RECV__NONE;

		pSvcParms->ReturnValue = TRUE;
	}
	*/

	uRecvLen = USB_GET_RX_BUFFER_COUNT(spUsbBuff->Ep3RxLength);
	uLen = (((UINT32_T) pSvcParms->Params[1]) > uRecvLen) ? uRecvLen : ((UINT32_T) pSvcParms->Params[1]);

	NMemCopyFromPaddedBuffer((UINT8_PTR_T) pSvcParms->Params[0], spRxBuf, uLen);
	if (pSvcParms->Params[2] != 0) {
		*((UINT32_PTR_T) pSvcParms->Params[2]) = uLen;
	}

	suRecvFlags = USB_RECV__NONE;

	pSvcParms->ReturnValue = TRUE;

	pTh->State = THREAD_STATE__WAITING;

	return TRUE;
}

void UsbCancelReceive()
{
	PTHREAD			pTh;
	PINITIAL_THREAD_STACK	pInitTh;

	suRecvFlags = USB_RECV__NONE;

	pTh = ThdGetWaitingInterrupt(0, THREAD_FLAG__IO_RECEIVE);
	if (!pTh) return;

	pTh->Flags &= ~(THREAD_FLAG__IO_RECEIVE);
	if (pTh->State != THREAD_STATE__SLEEP) 
		return;

	pInitTh = (PINITIAL_THREAD_STACK) pTh->LastStackPtr;
	pInitTh->ExcFrame.PC = pTh->SleepPC;

	pTh->LastError = ERR__USB_IO_CANCELLED;
	pTh->State = THREAD_STATE__WAITING;
}

BOOL UsbIsReady()
{
	return ((sControlLineState == 0x3) ? TRUE : FALSE);
}