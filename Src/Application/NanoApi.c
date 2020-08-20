/*
 * File    : NanoOSApi.c
 * Remark  : Implementation of NanoOS API and some helper functions.
 *
 */

#include "NanoOSApi.h"

extern unsigned int	_etext, _start_data, _end_data;
extern unsigned int	_start_bss, _end_bss;
unsigned char		__start_global_object_init, __end_global_object_init;

typedef void (* GLOB_OBJ_CTOR)();
typedef void (* GLOB_OBJ_DTOR)();

typedef struct _DTOR_ITEM
{
	void *			ObjectPtr;
	GLOB_OBJ_DTOR	Destructor;
} DTOR_ITEM, *PDTOR_ITEM;

extern int main(int argc, char *argv[]);

static const char	sHexCharsU[] = { "0123456789ABCDEF" };
static const char	sHexCharsL[] = { "0123456789abcdef" };

PDTOR_ITEM __dso_handle = 0;	// Use __dso_handle as a desctructor list pointer.

void __aeabi_atexit(void * pObj, GLOB_OBJ_DTOR dTor, void * pEndAddr)
{
	if ((unsigned int) __dso_handle > (APP_SRAM_LIMIT_ADDRESS - sizeof(DTOR_ITEM)))
		return;

	__dso_handle->ObjectPtr = pObj;
	__dso_handle->Destructor = dTor;
	__dso_handle++;
}

APP_SEGMENT_ATTR
int NanoEntry(void * pParm)
{
	unsigned int *		puiDst = 0;
	unsigned int *		puiSrc = 0;
	int					iRet = 0;

	/*
	 * Copy initialization value data from FLASH to data segment at RAM
	 */
	for (puiDst = &_start_data, puiSrc = &_etext; puiDst < &_end_data; )
	{
		*puiDst++ = *puiSrc++;
	}

	/*
	 * Zero bss segment at RAM
	 */
	for (puiDst = &_start_bss; puiDst < &_end_bss; ) 
	{
		*puiDst++ = 0;
	}

	__dso_handle = (PDTOR_ITEM) &_end_bss;

	/*
	 * Initialize CPP global object if any
	 */
	unsigned char *p = &__start_global_object_init;
	GLOB_OBJ_CTOR	GlobalObjCTor;
	while (p < &__end_global_object_init) {
		GlobalObjCTor = (GLOB_OBJ_CTOR) (p + 1);
		p += 0x10;

		(* GlobalObjCTor)();
	}

	/*
	 * Call main() function
	 */
	iRet = main(0, 0);

	/*
	 * De-initialize CPP global object if any
	 */
	while ((unsigned int) __dso_handle > (unsigned int) &_end_bss)
	{
		__dso_handle--;
		__asm volatile
			(
			"MOV.W R1, %0;"
			"MOV.W R0, %1;"
			"BLX R1;"
			: : "r" (__dso_handle->Destructor), "r" (__dso_handle->ObjectPtr)
			);
	}

	return iRet;
}

/*
 * Begin NanoOS API
 */

BOOL GetOSVersion(_OUT UINT32_PTR_T puiVersion)
{
	BOOL	fRes = 0;

	if (!puiVersion) {
		return FALSE;
	}

	CALL_SVC_WITH_ONE_PARAM(SVC__GET_OS_VERSION, fRes, puiVersion);

	return fRes;
}

BOOL GetOSName(_OUT INT8_PTR_T pBuffer, _IN UINT32_T uiBufferLength)
{
	BOOL	fRes = 0;

	if ((!pBuffer) || (uiBufferLength == 0)) {
		return FALSE;
	}

	CALL_SVC_WITH_TWO_PARAMS(SVC__GET_OS_NAME, fRes, pBuffer, uiBufferLength);

	return fRes;
}

void DebugCharOut(_IN UINT8_T uiData)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__DEBUG_CHAR_OUT, fRes, uiData);
}

BOOL GetSystemClockSpeed(_OUT UINT32_PTR_T puiClock)
{
	BOOL	fRes = 0;

	if (!puiClock) return FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__GET_SYSTEM_CLOCK, fRes, puiClock);

	return fRes;
}

BOOL IsUsbSerialReady()
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ZERO_PARAM(SVC__IS_USB_SER_READY, fRes);

	return fRes;
}

BOOL WriteToUsbSerial(_IN UINT8_PTR_T puiBuffer, _IN UINT32_T uiBufferLength)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_TWO_PARAMS(SVC__WRITE_USB_SER, fRes, puiBuffer, uiBufferLength);

	return fRes;
}

BOOL ReadFromUsbSerial(_OUT UINT8_PTR_T puiBuffer, _IN UINT32_T uiBufferLength, _OUT_OPT UINT32_PTR_T puiReadLength)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_THREE_PARAMS(SVC__READ_USB_SER, fRes, puiBuffer, uiBufferLength, puiReadLength);

	return fRes;
}

void CancelReadFromUsbSerial()
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ZERO_PARAM(SVC__CANCEL_READ_USB_SER, fRes);
}

BOOL CreateThread(
				  _IN UINT8_T uiControl, 
				  _IN BOOL fIsSuspended, 
				  _IN THREAD_ENTRY_TYPE Entry, 
				  _IN_OPT void * pParam, 
				  _OUT_OPT UINT32_PTR_T pTID
				  )
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_FIVE_PARAMS(
							SVC__CREATE_THREAD, 
							fRes, 
							uiControl, 
							fIsSuspended, 
							Entry, 
							pParam, 
							pTID
							);

	return fRes;
}

BOOL TerminateThread(_IN UINT32_T uiTID)
{
	BOOL fRes = 0;

	CALL_SVC_WITH_ONE_PARAM(SVC__TERMINATE_THREAD, fRes, uiTID);

	return fRes;
}

BOOL GetThreadStatus(_IN UINT32_T uiTID, _OUT UINT8_PTR_T puiStatus)
{
	BOOL fRes = 0;

	CALL_SVC_WITH_TWO_PARAMS(SVC__GET_THREAD_STATUS, fRes, uiTID, puiStatus);

	return fRes;
}

BOOL GetThreadReturnValue(_IN UINT32_T uiTID, _OUT int * piVal)
{
	BOOL fRes = 0;

	if (!piVal) return FALSE;

	CALL_SVC_WITH_TWO_PARAMS(SVC__GET_THREAD_RETURN_VALUE, fRes, uiTID, piVal);

	return fRes;
}

void ExitThread()
{
	BOOL	fRes = 0;

	CALL_SVC_WITH_ZERO_PARAM(SVC__EXIT_THREAD, fRes);
}

BOOL GetCurrentThreadId(_IN UINT32_PTR_T puiTID)
{
	BOOL fRes = FALSE;

	if (!puiTID) return FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__GET_CURRENT_THREAD_ID, fRes, puiTID);

	return fRes;
}

BOOL Sleep(_IN UINT32_T uiMiliSec)
{
	BOOL fRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__SLEEP, fRes, uiMiliSec);

	return fRes;
}

BOOL SuspendThread(_IN UINT32_T uiThreadId)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__SUSPEND_THREAD, fRes, uiThreadId);

	return fRes;
}

BOOL ResumeThread(_IN UINT32_T uiThreadId)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__RESUME_THREAD, fRes, uiThreadId);

	return fRes;
}

BOOL WaitForObject(
				   _IN UINT8_T uiObjectType, 
				   _IN UINT32_T uiObjectId, 
				   _IN UINT32_T uiMilliSec
				   )
{
	BOOL fRes = FALSE;

	CALL_SVC_WITH_THREE_PARAMS(
							SVC__WAIT_FOR_OBJECT, 
							fRes, 
							uiObjectType, 
							uiObjectId, 
							uiMilliSec
							);

	return fRes;
}

BOOL CreateEvent(_OUT UINT32_PTR_T puiEventId)
{
	BOOL fRes = FALSE;

	if (!puiEventId) return FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__CREATE_EVENT, fRes, puiEventId);

	return fRes;
}

BOOL SetEvent(_IN UINT32_T uiEventId)
{
	BOOL fRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__SET_EVENT, fRes, uiEventId);

	return fRes;
}

BOOL ResetEvent(_IN UINT32_T uiEventId)
{
	BOOL fRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__RESET_EVENT, fRes, uiEventId);

	return fRes;
}

BOOL CloseEvent(_IN UINT32_T uiEventId)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__CLOSE_EVENT, fRes, uiEventId);

	return fRes;
}

BOOL SetTerminationHandler(_IN TERMINATION_HANDLER_TYPE TermHandler)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__SET_TERMINATION_HANDLER, fRes, TermHandler);

	return fRes;
}

BOOL GetFaultAddress(_IN UINT32_T uiFault, _OUT UINT32_PTR_T puiBusFaultAddress, _OUT UINT32_PTR_T puiMemFaultAddress)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_THREE_PARAMS(SVC__GET_FAULT_ADDRESS, fRes, uiFault, puiBusFaultAddress, puiMemFaultAddress);

	return fRes;
}

UINT32_T GetLastFault()
{
	UINT32_T	uFault = 0;

	CALL_SVC_WITH_ZERO_PARAM(SVC__GET_LAST_FAULT, uFault);

	return uFault;
}

BOOL SwitchToNextThread()
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ZERO_PARAM(SVC__SWITCH_TO_NEXT_THREAD, fRes);

	return fRes;
}

void SetLastError(_IN UINT32_T uiError)
{
	BOOL	fRes;

	CALL_SVC_WITH_ONE_PARAM(SVC__SET_LAST_ERROR, fRes, uiError);
}

UINT32_T GetLastError()
{
	UINT32_T	uRes = 0;

	CALL_SVC_WITH_ZERO_PARAM(SVC__GET_LAST_ERROR, uRes);

	return uRes;
}

BOOL SetInterruptHandler(_IN INTERRUPT_HANDLER_TYPE IntHandler)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__SET_INT_HANDLER, fRes, (UINT32_T) IntHandler);

	return fRes;
}

/*
BOOL SetUsbInterruptHandler(_IN INTERRUPT_HANDLER_TYPE UsbIntHandler)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__SET_USB_INT_HANDLER, fRes, (UINT32_T) UsbIntHandler);

	return fRes;
}
*/

BOOL LoadConfig(_OUT PCONFIG pConfig)
{
	BOOL	fRes = FALSE;

	if (!pConfig) return FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__LOAD_CONFIG, fRes, pConfig);

	return fRes;
}

BOOL SaveConfig(_IN PCONFIG pConfig)
{
	BOOL	fRes = 0;

	if (!pConfig) return FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__SAVE_CONFIG, fRes, pConfig);

	return fRes;
}

BOOL UnlockFlash()
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ZERO_PARAM(SVC__UNLOCK_FLASH, fRes);

	return fRes;
}

BOOL LockFlash()
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ZERO_PARAM(SVC__LOCK_FLASH, fRes);

	return fRes;
}

BOOL EraseFlash(_IN UINT32_T uiStartPageNo, _IN UINT32_T uiNumberOfPages)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_TWO_PARAMS(SVC__ERASE_FLASH, fRes, uiStartPageNo, uiNumberOfPages);

	return fRes;
}

BOOL ProgramFlash(_IN UINT16_PTR_T puiDestinationAddress, _IN UINT16_PTR_T puiSourceAddress, _IN UINT32_T uiLength)
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_THREE_PARAMS(SVC__PROGRAM_FLASH, fRes, puiDestinationAddress, puiSourceAddress, uiLength);

	return fRes;
}

void InitializeSpinLock(_OUT UINT32_PTR_T pLock)
{
	if (!pLock) return;
	*pLock = 0;
}

BOOL AcquireSpinLock(_IN UINT32_PTR_T pLock)
{
	BOOL	fRes = FALSE;

	if (!pLock) return FALSE;

	do {
		CALL_SVC_WITH_ONE_PARAM(SVC__ACQUIRE_SPIN_LOCK, fRes, pLock);
	} while (!fRes);

	return fRes;
}

BOOL ReleaseSpinLock(_IN UINT32_PTR_T pLock)
{
	BOOL	fRes = FALSE;

	if (!pLock) return FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__RELEASE_SPIN_LOCK, fRes, pLock);

	return fRes;
}

void ResetSystem()
{
	BOOL	fRes = FALSE;

	CALL_SVC_WITH_ZERO_PARAM(SVC__RESET_SYSTEM, fRes);
}

/*
UINT32_T SetProbe(_IN UINT32_PTR_T puiCurrentStack, _IN UINT32_PTR_T puiCurrentFrameAddress)
{
	UINT32_T		uRes = 0;

	CALL_SVC_WITH_TWO_PARAMS(SVC__SET_PROBE, uRes, puiCurrentStack, puiCurrentFrameAddress);

	return uRes;
}
*/

__attribute__((naked))
UINT32_T SetProbe()
{
	__asm volatile 
		(
		"MOV.W R0, %0;"
		"SVC #0;"
		"MOV.W R0, R1;"
		"BX LR;"
		: : "r" (SVC__SET_PROBE)
		);
}

UINT32_T ResetProbe(_IN BOOL fIsReturn)
{
	UINT32_T	uRes = FALSE;

	CALL_SVC_WITH_ONE_PARAM(SVC__RESET_PROBE, uRes, fIsReturn);

	return uRes;
}

/*
 * End NanoOS API
 */


/*
 * Helper functions
 */

void MemSet(UINT8_PTR_T pBuffer, UINT8_T uVal, UINT32_T uiBufferLength)
{
	UINT32_T		ui;

	SetLastError(ERR__NONE);

	if ((!pBuffer) || (uiBufferLength == 0)) {
		SetLastError(ERR__INVALID_PARAMETER);
		return;
	}

	for (ui = 0; ui < uiBufferLength; ui++) {
		*pBuffer++ = uVal;
	}
}

int MemCopy(UINT8_PTR_T pDest, const UINT8_PTR_T pSrc, UINT32_T uiDestLength, UINT32_T uiSrcLength)
{
	UINT32_T		uMin;
	UINT8_PTR_T		pS = (UINT8_PTR_T) pSrc;

	if ((pDest == 0) || (pSrc == 0) || (uiDestLength == 0) || (uiSrcLength == 0))
		return -1;

	uMin = (uiDestLength > uiSrcLength) ? uiSrcLength : uiDestLength;

	for (UINT32_T u = 0; u < uMin; u++) {
		*pDest++ = *pS++;
	}

	return uMin;
}

int StrLen(const char * pStr)
{
	int	iRes = 0;
	char *	p = (char *) pStr;

	if (p == 0) return -1;

	while (*p != '\0')
	{
		p++;
		iRes++;
	}

	return iRes;
}

int StrCmp(const char * pStr1, const char * pStr2)
{
	int		iLen1 = StrLen(pStr1);
	int		iLen2 = StrLen(pStr2);
	int		iLen = 0;
	int		i = 0;
	char *	p1 = (char *) pStr1;
	char *	p2 = (char *) pStr2;

	if ((iLen1 <= 0) || (iLen2 <= 0)) return -1;

	if (iLen1 <= iLen2)
		iLen = iLen1;
	else
		iLen = iLen2;

	for (i = 0; i < iLen; i++)
	{
		if (*p1 != *p2) break;
		p1++;
		p2++;
	}

	if ((iLen1 == iLen2) && (i == iLen))
		return 0;

	return iLen;
}

int StrCopy(char * pBuffer, const char * pStr, int iBufferLen)
{
	int		iLen = StrLen(pStr);
	int		i;
	char *	pDst;
	char *	pSrc;

	if (iLen <= 0) return 0;
	if (iBufferLen <= 0) return 0;
	if (pBuffer == 0) return 0;

	iLen = ((iLen > iBufferLen) ? iLen : iBufferLen);
	if (iLen <= 1) return 0;

	iLen--;
	pDst = pBuffer;
	pSrc = (char *) pStr;
	for (i = 0; i < iLen; i++) {
		*pDst++ = *pSrc++;
	}

	return iLen;
}

BOOL GetFlashAddressFromPageNo(_IN UINT32_T uiPageNo, _OUT UINT32_PTR_T *ppuiFlashAddress)
{
	if (uiPageNo > MAX_FLASH_PAGE_NO) {
		SetLastError(ERR__INVALID_FLASH_PAGE_NO);
		return FALSE;
	}

	if (!ppuiFlashAddress) {
		SetLastError(ERR__INVALID_PARAMETER);
		return FALSE;
	}

	*ppuiFlashAddress = 
		(UINT32_PTR_T) (FLASH_WRITE_BASE_ADDRESS + (uiPageNo + FLASH_PAGE_OFFSET));

	return TRUE;
}

BOOL GetFlashPageNoFromAddress(_IN UINT32_PTR_T puiFlashAddress, _OUT UINT32_PTR_T puiPageNo)
{
	UINT32_T	u;
	UINT32_PTR_T	puLow, puHigh;

	if (!puiPageNo) {
		SetLastError(ERR__INVALID_PARAMETER);
		return FALSE;
	}

	for (u = 0; u <= MAX_FLASH_PAGE_NO; u++)
	{
		puLow = (UINT32_PTR_T) (FLASH_WRITE_BASE_ADDRESS + (u * FLASH_PAGE_OFFSET));
		puHigh = (UINT32_PTR_T) (FLASH_WRITE_BASE_ADDRESS + (((u + 1) * FLASH_PAGE_OFFSET) - 1));
		if ((puiFlashAddress >= puLow) && (puiFlashAddress <= puHigh)) 
			break;
	}

	if (u == (MAX_FLASH_PAGE_NO + 1)) {
		SetLastError(ERR__NO_FLASH_PAGE_FOUND);
		return FALSE;
	}

	*puiPageNo = u;

	return TRUE;
}

static BOOL DbgCOut(UINT8_T Char, void * pParm)
{
	DebugCharOut(Char);
	return TRUE;
}

UINT32_T DbgPrintf(const char *szFormat, ...)
{
	UINT32_T	uRet = 0;
	va_list		argList;

	va_start(argList, szFormat);
	uRet = UtlVPrintf(DbgCOut, 0, szFormat, argList);
	va_end(argList);

	return uRet;
}

static BOOL WriteChar(UINT8_T Char, void * pParm)
{
	UINT8_T		uBuf = Char;
	return WriteToUsbSerial(&uBuf, 1);
}

UINT32_T Printf(const char *szFormat, ...)
{
	UINT32_T	uRet = 0;
	va_list		argList;

	va_start(argList, szFormat);
	uRet = UtlVPrintf(WriteChar, 0, szFormat, argList);
	va_end(argList);

	return uRet;
}

UINT32_T UtlWriteString(PRINT_CHAR_CALLBACK PrintChar, void * pParam, const char *szString)
{
	UINT32_T		i = 0;
	UINT8_PTR_T		p = (UINT8_PTR_T) szString;

	if ((!PrintChar) || (!p)) return 0;

	while (*p != '\0') {
		(* PrintChar)(*p, pParam);
		i++;
		p++;
	}

	return i;
}

UINT32_T UtlWriteNumber(PRINT_CHAR_CALLBACK PrintChar, void * pParm, UINT32_T uNumber)
{
	//
	// Assume unsinged int of uNumber has 32 bits long
	//

	unsigned int	uDiv = 0, uMod = 0;
	char			buf[128];
	char *			pc;
	int				iRes = 0;
	BOOL			bIsNegative = FALSE;

	if (!PrintChar) return 0;

	if ((uNumber & BITHEX_31) == BITHEX_31) {
		bIsNegative = TRUE;
		uNumber = ~uNumber;
		uNumber++;
	}

	buf[0] = 0;
	pc = &buf[1];

	do {
		uDiv = uNumber / 10;
		uMod = uNumber - (uDiv * 10);
		*pc++ = sHexCharsU[uMod];
		uNumber = uDiv;
	} while (uDiv >= 10);
	if (uDiv != 0)
		*pc = sHexCharsU[uDiv];
	else
		pc--;

	if (bIsNegative) 
		(* PrintChar)('-', pParm);

	while (*pc != '\0') {
		(* PrintChar)(*pc, pParm);
		pc--;
		iRes++;
	}

	return (UINT32_T) iRes;
}

UINT32_T UtlWriteHexa(PRINT_CHAR_CALLBACK PrintChar, void * pParm, UINT8_T uIsHexCapital, UINT32_T uHex, UINT32_T uDigit)
{
	//
	// Assume unsigned int of uiHex has 32 bits long = 8 nibbles = 8 digit
	//

	char			buf[8];
	char			*pc;
	unsigned int	i, uMax;
	int				ch;

	if (!PrintChar) return 0;

	uMax = (uDigit < sizeof(buf)) ? uDigit : sizeof(buf);

	pc = &buf[0];
	for (i = 0; i < uMax; i++) {
		ch = uHex & 0xF;
		if (uIsHexCapital) {
			*pc++ = sHexCharsU[ch];
		} else {
			*pc++ = sHexCharsL[ch];
		}
		uHex >>= 4;
	}

	pc--;

	for (i = 0; i < uMax; i++) {
		(* PrintChar)(*pc, pParm);
		pc--;
	}

	return i;
}

UINT32_T UtlVPrintf(PRINT_CHAR_CALLBACK PrintChar, void * pPrintCharParam, const char *szFormat, va_list argList)
{
	char *			pc = (char *) szFormat;
	UINT32_T		u;
	UINT8_T			c;
	char *			s;
	UINT32_T		uRes = 0;
	UINT32_T		uCount = 0;
	UINT32_T		uDigit;
	BOOL			fRes = FALSE;

	if (pc == 0) return 0;
	if (PrintChar == 0) return 0;

	while (*pc != '\0')
	{
		c = *pc++;
		if (c != '%')
		{
			fRes = (* PrintChar)(c, pPrintCharParam);
			if (!fRes) break;
			uCount++;
		}
		else
		{
			switch ((int) *pc)
			{
			case 'd':
			case 'D':
				u = va_arg(argList, UINT32_T);
				uRes = UtlWriteNumber(PrintChar, pPrintCharParam, u);
				uCount += uRes;
				break;

			case 'l':
				UtlWriteString(PrintChar, pPrintCharParam, "['long long' is not supported yet]");
				break;

			case 'c':
			case 'C':
				c = (UINT8_T) va_arg(argList, UINT32_T);
				(* PrintChar)(c, pPrintCharParam);
				uCount++;
				break;

			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				uDigit = (UINT32_T) *pc - 48;
				pc++;
				u = (UINT32_T) va_arg(argList, UINT32_T);
				if (*pc == 'x') {
					uRes = UtlWriteHexa(PrintChar, pPrintCharParam, FALSE, u, uDigit);
					uCount += uRes;
				} else if (*pc == 'X') {
					uRes = UtlWriteHexa(PrintChar, pPrintCharParam, TRUE, u, uDigit);
					uCount += uRes;
				} else {
					uRes = UtlWriteString(PrintChar, pPrintCharParam, "[Invalid digit number of ");
					uCount += uRes;

					(* PrintChar)(*pc, pPrintCharParam);
					uCount++;
					
					(* PrintChar)(']', pPrintCharParam);
					uCount++;
				}
				break;

			case 'x':
				u = (UINT32_T) va_arg(argList, UINT32_T);
				uRes = UtlWriteHexa(PrintChar, pPrintCharParam, FALSE, u, 8);
				uCount += uRes;
				break;

			case 'X':
				u = (UINT32_T) va_arg(argList, UINT32_T);
				uRes = UtlWriteHexa(PrintChar, pPrintCharParam, TRUE, u, 8);
				uCount += uRes;
				break;

			case 's':
				s = va_arg(argList, INT8_PTR_T);
				uRes = UtlWriteString(PrintChar, pPrintCharParam, s);
				uCount += uRes;
				break;

			case '%':
				c = (UINT8_T) va_arg(argList, UINT32_T);
				(* PrintChar)(c, pPrintCharParam);
				uCount++;
				break;


			default:
				uRes = UtlWriteString(PrintChar, pPrintCharParam, "[Unknown format: ");
				uCount += uRes;

				(* PrintChar)(*pc, pPrintCharParam);
				uCount++;
				
				(* PrintChar)(']', pPrintCharParam);
				uCount++;
				break;
			}

			pc++;
		}
	}

	return uCount;
}

static BOOL StreamChar(UINT8_T Char, void * pParm)
{
	PSTREAM_PRINTF_PARAMS	pStrParm = (PSTREAM_PRINTF_PARAMS) pParm;

	if ((!pParm) || 
		(pStrParm->Buffer == 0) ||
		(pStrParm->BufferLength == 0)) 
	{
		return FALSE;
	}

	if (pStrParm->Counter >= pStrParm->BufferLength) return FALSE;

	*(pStrParm->Buffer) = Char;
	pStrParm->Buffer++;
	pStrParm->Counter++;

	return TRUE;
}

UINT32_T StreamPrintf(UINT8_PTR_T pBuffer, UINT32_T uBufferLength, const char * szFormat, ...)
{
	va_list					argList;
	STREAM_PRINTF_PARAMS	StrParm;

	if ((!pBuffer) ||
		(uBufferLength == 0))
	{
		return 0;
	}

	MemSet(pBuffer, 0, uBufferLength);

	StrParm.Buffer = pBuffer;
	StrParm.BufferLength = (uBufferLength - 1);	// Make sure reserve one byte for null terminated string.
	StrParm.Counter = 0;

	va_start(argList, szFormat);
	UtlVPrintf(StreamChar, (void *) &StrParm, szFormat, argList);
	va_end(argList);

	return StrParm.Counter;
}

/*
 * C++ compiler required functions
 */

/*
 * Not sure exactly when this function being called,
 * for now just outputing to debug iface and exit.
 */
void __cxa_pure_virtual()
{
	DbgPrintf("%s\r\n", __FUNCTION__);
	ExitThread();
}

void *memcpy(void * pDst, const void * pSrc, unsigned int uiLength)
{
	unsigned char *pD = (unsigned char *) pDst;
	unsigned char *pS = (unsigned char *) pSrc;
	for (unsigned int u = 0; u < uiLength; u++)
	{
		*pD++ = *pS++;
	}

	return pDst;
}