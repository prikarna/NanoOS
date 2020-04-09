/*
 * File    : NanoOSApi.h
 * Remark  : The definition of functions 'exported' by NanoOS. 
 *           Provide a safer access to NanoOS resources in multithread environment.
 *
 *           Function parameters tag:
 *           _IN      -> as input parameter and required
 *           _OUT     -> as output parameter and required
 *           _IN_OPT  -> as input parameter but optional
 *           _OUT_OPT -> as output parameter but optional
 *
 */

#ifndef NANOOS_API_H
#define NANOOS_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\Sys\CortexM\Asm.h"
#include "..\Sys\Type.h"
#include "..\ServiceDef.h"

#define APP_SEGMENT_ATTR			__attribute__ ((section(".nanoos_application")))

#define OS_MAJOR_VER(OSVersion)			((OSVersion >> 8) & 0xFF)
#define OS_MINOR_VER(OSVersion)			(OSVersion & 0xFF)

/*
 BOOL GetOSVersion(_OUT UINT32_PTR_T puiVersion)

 Desc.:
     Get OS version information.
 Params. :
     puiVersion
         Pointer to UINT32_T variable that receive version information. Use macro of OS_MAJOR_VER() and 
		 OS_MINOR_VER() to extract major and minor version respectively of returned version value.
 Return value : 
     TRUE if success or FALSE if fail.
 Error code : 
     ERR__INVALID_RETURN_VALUE_POINTER when puiVersion is equal to 0 or NULL.
 */
BOOL GetOSVersion(_OUT UINT32_PTR_T puiVersion);


/*
 BOOL GetOSName(_OUT INT8_PTR_T pBuffer, _IN UINT32_T uiBufferLength)
 
 Desc.:
     Get OS name information.
 Params. :
     pBuffer
         Pointer to buffer that receive name information.
	 uiBufferLength
         Input buffer (pBuffer) length.
 Return value :
     TRUE if success or FALSE if fail.
 Error code : 
     ERR__INVALID_RETURN_VALUE_POINTER when pBuffer or uiBufferLength is equal to 0 or NULL
	 ERR__INSUFFICIENT_BUFFER when buffer length is not sufficient.
 */
BOOL GetOSName(_OUT INT8_PTR_T pBuffer, _IN UINT32_T uiBufferLength);


/*
 BOOL GetSystemClockSpeed(_OUT UINT32_PTR_T puiClock)
 
 Desc.:
     Get system clock speed in Herzt (Hz).
 Params. :
     puiClock
         Pointer to UINT32_T variable that receive clock speed information.
 Return value : 
     TRUE if success or FALSE if fail.
 Error code : 
     ERR__INVALID_RETURN_VALUE_POINTER when puiClock is equal to 0 or NULL.
 */
BOOL GetSystemClockSpeed(_OUT UINT32_PTR_T puiClock);


/*
 void DebugCharOut(_IN UINT8_T uiData)
 
 Desc.:
     Printf a character in debug output (Serial TTL).
 Params. :
     uiData
         Character to sent to debug output.
 Return value : 
     None.
 Error code : 
     None.
 */
void DebugCharOut(_IN UINT8_T uiData);


/*
 BOOL WriteToUsbSerial(_IN UINT8_PTR_T puiBuffer, _IN UINT32_T uiBufferLength)
 
 Desc.:
     Write data or buffer pointed by puiBuffer to USB Serial with length of uiBufferLength. The max. data
	 length to write to is USB_DATA_SIZE - 2 (62 bytes). If data length is higher than that the result may 
	 unpredictable. Note that to the receiving side point of view, this is a serial (COM) port, and PC 
	 application should treat this as serial (COM) port.
	 This operation is synchronous and almost always success when USB Serial is ready.
 Params. :
     puiBuffer
         Pointer to data or buffer to be written to.
     uiBufferLength
         The length of data or buffer (puiBuffer). Max. data length is USB_DATA_SIZE - 2 (62 bytes).
 Return value : 
     TRUE if success or FALSE if fail.
 Error code : 
     ERR__USB_IO_IS_NOT_READY when USB Serial is not ready to read or write operation.
     ERR__INVALID_PARAMETER when puiBuffer or uiBufferLength is equal to 0 (NULL).
	 ERR__USB_IO when operation is aborted or when USB protocol error occur.
 */
BOOL WriteToUsbSerial(_IN UINT8_PTR_T puiBuffer, _IN UINT32_T uiBufferLength);


/*
 BOOL ReadFromUsbSerial(_OUT UINT8_PTR_T puiBuffer, _IN UINT32_T uiBufferLength, _OUT_OPT UINT32_PTR_T puiReadLength)
 
 Desc.:
     Read from USB Serial and the result is copied to buffer pointed by puiBufferLength with length of uiBufferLength.
     If puiReadLength is given then variable pointed by puiReadLength will contain data length that being copied to
     the buffer. If uiBufferLength is more than USB_DATA_SIZE (64 bytes) then data will be truncated (USB Serial will 
	 use USB_DATA_SIZE to read data).
	 This operation is synchronous and can be cancelled via CancelReadFromUsbSerial() from another thread or from 
	 termination handler (Please see SetTerminationHandler() for more).
	 Only one thread can call this function at a time.
	 Note that to the sending side (a PC) point of view, this is a serial (COM) port, and PC application should treat 
	 this as serial (COM) port.
 Params. :
     puiBuffer
         Pointer to data or buffer to receive data from USB Serial.
     uiBufferLength
         The length of data or buffer (puiBuffer).
     puiReadLength
         Pointer to variable that will receive the data length being read from USB Serial. This is optional.
 Return value : 
     TRUE if success or FALSE if fail.
 Error code : 
     ERR__USB_IO_IS_BUSY when another thread still use this function.
     ERR__USB_IO_IS_NOT_READY when USB Serial is not ready for read or write operation.
     ERR__INVALID_PARAMETER when puiBuffer or uiBufferLength is equal to 0 (NULL).
	 ERR__USB_IO when operation is aborted or cancelled or when USB protocol error occur.
 */
BOOL ReadFromUsbSerial(_OUT UINT8_PTR_T puiBuffer, _IN UINT32_T uiBufferLength, _OUT_OPT UINT32_PTR_T puiReadLength);


/*
 void CancelReadFromUsbSerial()
 
 Desc.:
     Cancel ReadFromUsbSerial() call from another thread or termination handler.
 Params. :
     None.
 Return value : 
     None.
 Error code : 
     None.
 */
void CancelReadFromUsbSerial();


/*
 BOOL IsUsbSerialReady()
 
 Desc.:
     To check whether USB Serial is ready or not.
 Params. :
     None.
 Return value : 
     TRUE if USB Serial is ready or otherwise FALSE.
 Error code : 
     None.
*/
BOOL IsUsbSerialReady();


#include "..\ThreadDef.h"

/*
 BOOL CreateThread(
				  _IN UINT8_T uiControl, 
				  _IN BOOL fIsSuspended, 
				  _IN THREAD_ENTRY_TYPE Entry, 
				  _IN_OPT void * pParam, 
				  _OUT_OPT UINT32_PTR_T pTID
				  )

 Desc.:
     Create new thread. For now only support 4 threads including the running one concurrently.
 Params. :
     uiControl
	     Thread control or privilege, currently supported control is the one of the following:
             THREAD_CONTROL__PRIVILEGED
			     Thread has access to Cortex-M peripherals (SCB, NVIC etc.)
             THREAD_CONTROL__UNPRIVILEGED
                 Thread has no access to Cortex-M peripherals.
     fIsSuspended
	     If TRUE thread will be created in suspended state. To resume this thread use ResumeThread(). Otherwise
		 thread will be executed as soon as it created.
     Entry
	     Funtion with the signature of THREAD_ENTRY_TYPE that will used as thread entry point (executed by processor
		 right after it created).
     pParam
         Pointer to a parameter that will be passed to Entry (thread entry point) function.
	 pTID
         Pointer to UINT32_T variable which will receive thread id. This optional. This thread id can be used to control
		 the thread, e.g., suspend or resume.
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
     ERR__INVALID_THREAD_ENTRY when Entry parameter is equal to 0 (NULL).
	 ERR__NO_MORE_THREAD_SLOT when no more thread slot available to create new thread (max. number of thread = 4).
	 ERR__INVALID_THREAD_CONTROL when uiControl parameter is not in the thread control list above.
 */
BOOL CreateThread(
				  _IN UINT8_T uiControl, 
				  _IN BOOL fIsSuspended, 
				  _IN THREAD_ENTRY_TYPE Entry, 
				  _IN_OPT void * pParam, 
				  _OUT_OPT UINT32_PTR_T pTID
				  );


/*
 BOOL TerminateThread(_IN UINT32_T uiTID)
 
 Desc.:
     Terminate another thread from current thread. Thread can terminate each other regardless of what the thread
	 control is. To terminate or exit current thread use ExitThread().
 Params. :
	 uiTID
         Thread id of the thread that will be terminated.
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
     ERR__INVALID_THREAD_ID when uiTID is current thread id or uiTID is invalid (more than max. number of thread).
	 ERR__INVALID_THREAD_STATE when another thread is in exit state.
 */
BOOL TerminateThread(_IN UINT32_T uiTID);


/*
 void ExitThread()
 
 Desc.:
     Exit or terminate current thread. To terminate another thread use TerminateThread().
 Params. :
	 None.
 Return value : 
     None.
 Error code : 
     None.
 */
void ExitThread();


/*
 BOOL GetThreadStatus(_IN UINT32_T uiTID, _OUT UINT8_PTR_T puiStatus)
 
 Desc.:
     Get status of thread specified thread id.
 Params. :
	 uiTID
         Thread id of the thread that will be queried.
	 puiStatus
         Pointer to UINT8_T variable that will receive the status thread indentified by uiTID. This is required,
		 can not be NULL (0).
		 If success this variable will contain of one of the following:
		     THREAD_STATUS__EXIT
			     Thread is exited and indicate that the thread reach the end of its execution point if the thread 
				 don't call ExitThread(). This status is also set when the thread call ExitThread().
			 THREAD_STATUS__TERMINATED
			     Thread is terminated and indicate that the thread does not reach the end of its execution point,
				 possibly terminated by other thread or fault(s) occur in the middle of its execution.
             THREAD_STATUS__ACTIVE
			     Thread is still active.
			 THREAD_STATUS__SUSPENDED
			     Thread is suspended.
			 THREAD_STATUS__SLEEP
			     Thread is sleeping. This occur when a thread call Sleep() or WaitForObject().
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
     ERR__INVALID_PARAMETER when puiStatus is equal to 0 (NULL).
	 ERR__INVALID_THREAD_ID when uiTID is invalid (more than max. number of thread).
 */
BOOL GetThreadStatus(_IN UINT32_T uiTID, _OUT UINT8_PTR_T puiStatus);


/*
 BOOL GetThreadReturnValue(_IN UINT32_T uiTID, _OUT int * piVal)
 
 Desc.:
     Get the return value of thread specified by uiTID.
 Params. :
	 uiTID
         Thread id of the thread that will be queried.
	 piVal
         Pointer to int variable that will receive a return value of specified thread id. This is required,
		 can not be NULL (0). The return value is valid when specified thread reach the end of its execution, 
		 thread changes status from THREAD_STATUS__ACTIVE to THREAD_STATUS__EXIT naturally without calling 
		 ExitThread().
		 For other status the return value is not valid even if this function return TRUE (success).
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
     ERR__INVALID_PARAMETER when piVal is equal to 0 (NULL).
	 ERR__INVALID_THREAD_ID when uiTID is invalid (more than max. number of thread).
 */
BOOL GetThreadReturnValue(_IN UINT32_T uiTID, _OUT int * piVal);


/*
 BOOL GetCurrentThreadId(_IN UINT32_PTR_T puiTID)
 
 Desc.:
     Get the current thread id.
 Params. :
	 puiTID
         Pointer to UINT32_T variable that will receive current thread id if this function is success. 
		 This is required, can not be NULL (0).
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
     ERR__INVALID_PARAMETER when puiTID is equal to 0 (NULL).
 */
BOOL GetCurrentThreadId(_IN UINT32_PTR_T puiTID);


/*
 BOOL Sleep(_IN UINT32_T uiMilliSec)
 
 Desc.:
     Sleep current thread at specified mili second of time (Note that is just approximation).
 Params. :
	 uiMilliSec
	     Specifiy how long the current thread will sleep in mili seconds. This value must be greater than
         THREAD__MIN_MSEC_SLEEP and less than THREAD__INFINITE_WAIT. Current thread can not sleep indefenitely.
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
     ERR__INVALID_MILISEC_VALUE 
         When uiMiliSec is lower than THREAD__MIN_MSEC_SLEEP or greater or equal to THREAD__INFINITE_WAIT.
 */
BOOL Sleep(_IN UINT32_T uiMilliSec);


/*
 BOOL SuspendThread(_IN UINT32_T uiThreadId)

 Desc.:
     Suspend current thread or another thread (specified by uiThreadId).
 Params. :
	 uiThreadId
	     Specifiy the thread id that will be suspended, can be current or another thread id.
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
     ERR__INVALID_THREAD_ID when specified thread id is invalid (more than max. number of thread).
	 ERR__INVALID_THREAD_STATE when thread specified by thread id is not active.
 */
BOOL SuspendThread(_IN UINT32_T uiThreadId);


/*
 BOOL ResumeThread(_IN UINT32_T uiThreadId)

 Desc.:
     Resume another suspended thread specified by uiThreadId.
 Params. :
	 uiThreadId
	     Specifiy the thread id that will be resumed from suspended.
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
     ERR__INVALID_THREAD_ID when specified thread id is invalid (more than max. number of thread).
	 ERR__INVALID_THREAD_STATE when thread specified by thread id is not in suspended state.
 */
BOOL ResumeThread(_IN UINT32_T uiThreadId);


/*
 BOOL WaitForObject(
				   _IN UINT8_T uiObjectType, 
				   _IN UINT32_T uiObjectId, 
				   _IN UINT32_T uiMilliSec
				   )
 
 Desc.:
     Current thread wait for specified object until specified time.
 Params. :
	 uiObjectType
         Currently supported object type is one of the following:
		     THREAD_WAIT_OBJ__EVENT
			     The object is event object, created by CreateEvent(). For this object, current thread will wait until
				 specified event object is set or uiMiliSec exceeded. When uiMiliSec exceeded the function will return 
				 FALSE and last error will be ERR__WAIT_TIMEOUT.
			 THREAD_WAIT_OBJ__THREAD
			     The object is thread object, created by CreateThread(). For this object, current thread will wait until
				 specified thread object has exited or terminated or uiMiliSec exceeded. When uiMiliSec exceeded the
				 function will return FALSE and last error will be ERR__WAIT_TIMEOUT.
     uiObjectId
	     Object id.
     uiMilliSec
	     Specifiy how long the current thread will wait in mili seconds. This value must be greater than
         THREAD__MIN_MSEC_SLEEP. Current thread can wait indefenitely by specify THREAD__INFINITE_WAIT in 
		 this parameter.
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
     ERR__INVALID_MILISEC_VALUE when uiMiliSec is lower than THREAD__MIN_MSEC_SLEEP.
	 ERR__EVENT_ALREADY_BEEN_SET when event object already set before thread wait. (Considered as error).
	 ERR__INVALID_THREAD_STATE when thread object already terminated.
	 ERR__INVALID_THREAD_WAIT_OBJ_TYPE when uiObjectType is not one of listed above.
	 ERR__WAIT_TIMEOUT when uiMiliSec wait time exceeded.
	 ERR__INVALID_EVENT_ID when object is event and event id is invalid.
	 ERR__INVALID_EVENT_STATE when object is event and event state is invalid.
	 Error code from GetThreadStatus() for thread object.
 */
BOOL WaitForObject(
				   _IN UINT8_T uiObjectType, 
				   _IN UINT32_T uiObjectId, 
				   _IN UINT32_T uiMilliSec
				   );


/*
 BOOL SwitchToNextThread()

 Desc.:
     Switch current thread to next thread choosen by thread mechanism.
 Params. :
	 None.
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
	 None.
 */
BOOL SwitchToNextThread();

/*
 BOOL CreateEvent(_OUT UINT32_PTR_T puiEventId)

 Desc.:
     Create new event object. Currently only 64 events can be created concurrently.
 Params. :
	 puiEventId
	     Pointer to UINT32_T variable that will receive event id when success. Can not be NULL (0).
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
     ERR__INVALID_PARAMETER when puiEventId is equal to 0 (NULL).
	 ERR__NO_MORE_EVENT_SLOT when no more event slot available to create new event.
 */
BOOL CreateEvent(_OUT UINT32_PTR_T puiEventId);

/*
 BOOL SetEvent(_IN UINT32_T uiEventId)

 Desc.:
     Set event specified by event id.
 Params. :
	 uiEventId
	     Specify event id to set to.
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
     ERR__INVALID_EVENT_ID when event id is equal or more than max. number of concurrent created event objects.
 */
BOOL SetEvent(_IN UINT32_T uiEventId);

/*
 BOOL ResetEvent(_IN UINT32_T uiEventId)

 Desc.:
     Reset (unset or clear) event specified by event id.
 Params. :
	 uiEventId
	     Specify event id to reset.
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
     ERR__INVALID_EVENT_ID when event id is equal or more than max. number of concurrent created event objects.
 */
BOOL ResetEvent(_IN UINT32_T uiEventId);

/*
 BOOL CloseEvent(_IN UINT32_T uiEventId)

 Desc.:
     Close (or release from being used) event specified by event id.
 Params. :
	 uiEventId
	     Specify event id to be closed.
 Return value : 
     TRUE if success or otherwise FALSE.
 Error code : 
     ERR__INVALID_EVENT_ID when event id is equal or more than max. number of concurrent created event objects.
 */
BOOL CloseEvent(_IN UINT32_T uiEventId);

/*
 * Termination reason list of terminating thread (thread being terminated)
 * When termination handler is called, its parameter contain one or more 
 * termination reason(s) as in the following:
 */
/* Usage fault */
#define TERM__NO_CO_PROCESSOR_FAULT				0x00080000
#define TERM__INVALID_PC_LOAD_FAULT				0x00040000
#define TERM__INVALID_STATE_FAULT				0x00020000
#define TERM__UNDEF_INSTRUCTION_FAULT			0x00010000

/* Bus fault */
#define TERM__BUS_EXC_ENTRY_STACKING_ERROR		0x00001000
#define TERM__BUS_EXC_RETURN_STACKING_ERROR		0x00000800
#define TERM__IMPRECISE_ERROR_DATA_FAULT		0x00000400
#define TERM__PRECISE_ERROR_DATA_FAULT			0x00000200
#define TERM__INSTRUCTION_ERROR_FAULT			0x00000100

/* Memory fault */
#define TERM__MEM_EXC_ENTRY_STACKING_ERROR			0x00000010
#define TERM__MEM_EXC_RETURN_STACKING_ERROR			0x00000008
#define TERM__DATA_ACCESS_VIOLATION_FAULT			0x00000002
#define TERM__INSTRUCTION_ACCESS_VIOLATION_FAULT	0x00000001

/*
 BOOL SetTerminationHandler(_IN TERMINATION_HANDLER_TYPE TermHandler)

 Desc.:
     Set termination handler to current thread. About termination handler please see the definition of 
	 TERMINATION_HANDLER_TYPE in ThreadDef.h
 Params. :
	 TermHandler
	     Specify a termination handler function with signature of TERMINATION_HANDLER_TYPE. This parameter
		 can be 0 (NULL) to remove currently installed termination handler.
 Return value : 
     TRUE is success or otherwise FALSE.
 Error code : 
     ERR__OPERATION_NOT_ALLOWED when call this function in termination handler.
 */
BOOL SetTerminationHandler(_IN TERMINATION_HANDLER_TYPE TermHandler);

/*
 BOOL GetFaultAddress(_IN UINT32_T uiFault, _OUT UINT32_PTR_T puiBusFaultAddress, _OUT UINT32_PTR_T puiMemFaultAddress)

 Desc.:
     Get the faulting address when a thread encounter fault(s).
 Params. :
	 uiFault
	     Specify fault(s) that occured in current thread, this value is obtained from termination handler parameter 
		 (Please see the termination handler signature / TERMINATION_HANDLER_TYPE) or from return value of GetLastFault().
	 puiBusFaultAddress
	     Pointer to UINT32_T variable that will receive bus faulting address or 0 if there is none.
     puiMemFaultAddress
	     Pointer to UINT32_T variable that will receive memory faulting address or 0 if there is none.
 Return value : 
     TRUE if success (if bus or memory or both faulting address exist) or otherwise FALSE.
 Error code : 
     None.
 */
BOOL GetFaultAddress(_IN UINT32_T uiFault, _OUT UINT32_PTR_T puiBusFaultAddress, _OUT UINT32_PTR_T puiMemFaultAddress);

/*
 UINT32_T GetLastFault()

 Desc.:
     Get last fault(s) of the global space.
 Params. :
	 None.
 Return value : 
     The last fault(s) that has been already occured.
 Error code : 
     None.
 */
UINT32_T GetLastFault();

#include "..\Error.h"

/*
 void SetLastError(_IN UINT32_T uiError)

 Desc.:
     Set current thread last error code.
 Params. :
	 uiError
	     Error code, can be one of error code listed in Error.h or application custom error code. Note that custom error
		 code should not use already defined error code in Error.h.
 Return value : 
     None.
 Error code : 
     None.
 */
void SetLastError(_IN UINT32_T uiError);

/*
 UINT32_T GetLastError()

 Desc.:
     Get current thread last error code.
 Params. :
     None.
 Return value : 
     Last error code can be error code defined in Error.h or application custom error code.
 Error code : 
     None.
 */
UINT32_T GetLastError();

#include "..\Interrupt.h"

/*
 BOOL SetInterruptHandler(_IN INTERRUPT_HANDLER_TYPE IntHandler)

 Desc.:
     Set global interrupt handler. This interrupt handler is global, application can distinguish the interrupt number
	 or type by checking interrupt handler parameter (Please see definition of interrupt handler / INTERRUPT_HANDLER_TYPE).
 Params. :
	 IntHandler
	     Interrupt handler or 0 (NULL) to remove currently installed interrupt handler.
 Return value : 
     TRUE.
 Error code : 
     None.
 */
BOOL SetInterruptHandler(_IN INTERRUPT_HANDLER_TYPE IntHandler);

#include "..\Config.h"

/*
 BOOL LoadConfig(_OUT PCONFIG pConfig)

 Desc.:
     Load current NanoOS configuration to application allocated CONFIG structure.
 Params. :
	 pConfig
	     Pointer to CONFIG structure variable that will receive current configuation.
 Return value : 
     TRUE if success otherwise FALSE.
 Error code : 
     ERR__INVALID_PARAMETER when pConfig is equal to 0 (NULL).
 */
BOOL LoadConfig(_OUT PCONFIG pConfig);

/*
 BOOL SaveConfig(_IN PCONFIG pConfig)

 Desc.:
     Save or write application allocated CONFIG structure to NanoOS current configuration.
 Params. :
	 pConfig
	     Pointer to CONFIG structure variable that will be saved or written to NanoOS current conifguration.
 Return value : 
     TRUE if success otherwise FALSE.
 Error code : 
     ERR__INVALID_PARAMETER when pConfig is equal to 0 (NULL).
	 ERR__FAIL_TO_UNLOCK_FLASH when fail to unlock flash memory.
	 ERR__FLASH_OPERATION when fail erase or write flash memory.
	 ERR__FLASH_PROGRAMMING when fail to write to flash memory.
	 ERR__FLASH_WRITE_PROTECTION when write protection error occur while write to flash memory.
 */
BOOL SaveConfig(_IN PCONFIG pConfig);

#define MAX_FLASH_PAGE_NO			127
#define FLASH_WRITE_BASE_ADDRESS	0x08000000
#define FLASH_PAGE_OFFSET			0x00000400				// 1 KB
#define FLASH_PAGE_SIZE				FLASH_PAGE_OFFSET		// 1 KB

/*
 BOOL GetFlashAddressFromPageNo(_IN UINT32_T uiPageNo, _OUT UINT32_PTR_T *ppuiFlashAddress)

 Desc.:
     Get flash memory address from page number. Flash page address is used to program or write to flash memory.
 Params. :
	 uiPageNo
	     Flash page number.
     ppuiFlashAddress
	     Pointer to UINT32_PTR_T variable that will receive flash memory address when success.
 Return value : 
     TRUE if success otherwise FALSE.
 Error code : 
     ERR__INVALID_PARAMETER when ppuiFlashAddress is equal to 0 (NULL).
	 ERR__INVALID_FLASH_PAGE_NO when page number exceeded the MAX_FLASH_PAGE_NO.
 */
BOOL GetFlashAddressFromPageNo(_IN UINT32_T uiPageNo, _OUT UINT32_PTR_T *ppuiFlashAddress);

/*
 BOOL GetFlashPageNoFromAddress(_IN UINT32_PTR_T puiFlashAddress, _OUT UINT32_PTR_T puiPageNo)

 Desc.:
     Get flash page number from flash memory address. Flash page number is used to erase flash memory.
 Params. :
	 puiFlashAddress
	     Flash memory address.
     puiPageNo
	     Pointer to UINT32_T variable that will receive flash page number when success.
 Return value : 
     TRUE if success otherwise FALSE.
 Error code : 
     ERR__INVALID_PARAMETER when puiPageNo is equal to 0 (NULL).
	 ERR__NO_FLASH_PAGE_FOUND when flash page number not found.
 */
BOOL GetFlashPageNoFromAddress(_IN UINT32_PTR_T puiFlashAddress, _OUT UINT32_PTR_T puiPageNo);

/*
 BOOL UnlockFlash()

 Desc.:
     Unlock flash memory. Use this function to unlock flash memory before erase and/or program (write) to
	 flash memory.
 Params. :
	 None.
 Return value : 
     TRUE if success otherwise FALSE.
 Error code : 
	 ERR__FAIL_TO_UNLOCK_FLASH when fail to unlock flash memory.
 */
BOOL UnlockFlash();

/*
 BOOL LockFlash()

 Desc.:
     Lock flash memory. Use this function to lock flash memory after erase and/or program (write) to flash
	 memory.
 Params. :
	 None.
 Return value : 
     TRUE if success otherwise FALSE.
 Error code : 
	 ERR__FAIL_TO_LOCK_FLASH when fail to lock flash memory.
 */
BOOL LockFlash();

/*
 BOOL EraseFlash(_IN UINT32_T uiStartPageNo, _IN UINT32_T uiNumberOfPages)

 Desc.:
     Erase flash memory start from specified page number in specified number of pages.
 Params. :
	 uiStartPageNo
	     Specify starting flash page number.
     uiNumberOfPages
	     Specify number of flash page(s) to be erased. 1 flash page is equal to 1 KB flash memory.
 Return value : 
     TRUE if success otherwise FALSE.
 Error code : 
     ERR__INVALID_FLASH_OPERATION_STATE when flash memory is locked.
	 ERR__INVALID_FLASH_PAGE_NO when page number exceeded the MAX_FLASH_PAGE_NO.
	 ERR__FLASH_OPERATION when fail erase flash memory.
 */
BOOL EraseFlash(_IN UINT32_T uiStartPageNo, _IN UINT32_T uiNumberOfPages);

/*
 BOOL ProgramFlash(
                   _IN UINT16_PTR_T puiDestinationAddress, 
				   _IN UINT16_PTR_T puiSourceAddress, 
				   _IN UINT32_T uiNumberOfOperation
				   )

 Desc.:
     Program or write data to flash memory to specified flash memory address from specified from specified source addres
	 with specified number proram or write operation.
 Params. :
	 puiDestinationAddress
	     Specify flash memory (destination) address to be programed or written to.
     puiSourceAddress
	     Specify source address of program or write flash operation.
     uiNumberOfOperation
	     Specify how many time the operation will run. Note that 1 operation program (write) 2 bytes data to flash memory
		 (destination).
 Return value : 
     TRUE if success otherwise FALSE.
 Error code : 
     ERR__INVALID_PARAMETER when destination address or source address is equal to 0 (NULL).
	 ERR__INVALID_FLASH_NUMBER_OF_OPERATION when uiNumberOfOperation is equal to 0.
     ERR__INVALID_FLASH_OPERATION_STATE when flash memory is locked.
	 ERR__INVALID_FLASH_PAGE_NO when page number exceeded the MAX_FLASH_PAGE_NO.
	 ERR__FLASH_PROGRAMMING when fail to program or write to flash memory.
	 ERR__FLASH_WRITE_PROTECTION when write protection error occur while program or write to flash memory.
	 ERR__FLASH_OPERATION when fail erase flash memory.
 */
BOOL ProgramFlash(
				  _IN UINT16_PTR_T puiDestinationAddress, 
				  _IN UINT16_PTR_T puiSourceAddress, 
				  _IN UINT32_T uiNumberOfOperation
				  );

/*
 void InitializeSpinLock(_OUT UINT32_PTR_T pLock)

 Desc.:
     Initialize spinlock with specified lock parameter.
 Params. :
	 pLock
	     Pointer to UINT32_T variable that will be initialized and used as lock. This is required. 
		 This variable must the same for InitializeSpinLock(), AcquireSpinLock() and ReleseSpinLock() sequence.
 Return value : 
     None.
 Error code : 
     None.
 */
void InitializeSpinLock(_OUT UINT32_PTR_T pLock);

/*
 BOOL AcquireSpinLock(_IN UINT32_PTR_T pLock)

 Desc.:
     Acquire spinlock with specified lock parameter.
 Params. :
	 pLock
	     Pointer to UINT32_T variable that will be acquired. This is required. This variable must the same for 
		 InitializeSpinLock(), AcquireSpinLock() and ReleseSpinLock() sequence.
 Return value : 
     TRUE if success otherwise FALSE (when pLock is equal to 0).
 Error code : 
     None.
 */
BOOL AcquireSpinLock(_IN UINT32_PTR_T pLock);

/*
 BOOL ReleaseSpinLock(_IN UINT32_PTR_T pLock)

 Desc.:
     Release spinlock with specified lock parameter.
 Params. :
	 pLock
	     Pointer to UINT32_T variable that will be release. This is required. This variable must the same for 
		 InitializeSpinLock(), AcquireSpinLock() and ReleseSpinLock() sequence.
 Return value : 
     TRUE if success otherwise FALSE (when pLock is equal to 0).
 Error code : 
     None.
 */
BOOL ReleaseSpinLock(_IN UINT32_PTR_T pLock);

/*
 void ResetSystem()

 Desc.:
     Reset the chip.
 Params. :
	 None.
 Return value : 
     None.
 Error code : 
     None.
 */
void ResetSystem();

/*
 UINT32_T SetProbe()

 Desc.:
     Set current thread to probe mode. In this mode, when fault occur NanoOS general fault handler will redirect
	 the thread execution point to where the SetProbe() is called and about to return and give the calling 
	 thread return value of PROBE_STATUS__FAULT. In this way a thread can 'simulate' or 'mimic' try/finally pair 
	 so this thread can choose to do whatever it needs without worried being terminated by NanoOS when encounter 
	 fault(s) (Note that by default NanoOS will terminate faulty thread).
 Params. :
	 None.
 Return value : 
     PROBE_STATUS__ERROR
	     When fail to set probe to current thread. SetProbe() can fail when thread is already in probe mode.
	 PROBE_STATUS__ORIGIN
	     Indicate that SetProbe() is success and this is 'first return' of the function.
	 PROBE_STATUS__FAULT
	     Indicate that the thread encounter fault(s) and this is 'secondary return', calling thread can do whatever 
		 it needs to do when SetProbe() return this value.
	 PROBE_STATUS__RETURN
	     Indicate that the other part of calling thread call ResetProbe(), this is usefull when thread want to 'simulate'
		 or 'mimic' set_jump/long_jump logic.
 Error code : 
     ERR__ALREADY_PROBING when probe is already being set in the current thread.
 */
UINT32_T SetProbe();

/*
 UINT32_T ResetProbe(_IN BOOL fIsReturn)

 Desc.:
     Unset or clear currently probing thread.
 Params. :
	 fIsReturn
	     When TRUE the calling thread will return to where the SetProbe() is called and about to return and give
         PROBE_STATUS__RETURN to the calling thread if success. Otherwise this function will return 
		 PROBE_STATUS__NO_RETURN if success.
 Return value : 
     PROBE_STATUS__ERROR
	     When current thread is not is probe mode or probe is not set in current thread.
	 PROBE_STATUS__NO_RETURN
	     When success and fIsReturn is equal to FALSE.
 Error code : 
     ERR__PROBE_NOT_SET when probe is not set in current thread.
 */
UINT32_T ResetProbe(_IN BOOL fIsReturn);

/*
 * Some helper functions
 */
/*
 void MemSet(UINT8_PTR_T pBuffer, UINT8_T uVal, UINT32_T uiBufferLength)

 Desc.:
     Set byte by byte memory pointed by pBuffer with uVal in length of uiBufferLength.
 Params. :
	 pBuffer
	     Pointer to memory or buffer that its data wil be set.
     uVal
	     A value to assign to buffer byte by byte.
     uiBufferLength
	     Length of the buffer or memory in bytes.
 Return value : 
     None.
 Error code : 
     ERR__INVALID_PARAMETER when pointer to memory (buffer) or buffer length is equal to 0.
 */
void MemSet(UINT8_PTR_T pBuffer, UINT8_T uVal, UINT32_T uiBufferLength);

/*
 int MemCopy(UINT8_PTR_T pDest, const UINT8_PTR_T pSrc, UINT32_T uiDestLength, UINT32_T uiSrcLength)

 Desc.:
     Copy byte by byte from memory pointed by pSrc to memory pointed by pDst. The length of data being copied is 
	 the minimum value of uiDestLength or uiSrcLength.
 Params. :
	 pDst
	     Pointer to memory or buffer that data to be copied to (destination).
     pSrc
	     Pointer to memory or buffer that data being copied from (source).
     uiDestLength
	     Length of the buffer or memory of destination.
	 uiSrcLength
	     Length of the buffer or memory of source.
 Return value : 
     -1 when pDst or pSrc or uiDstLength or uiSrcLength is equal to 0.
	 Number byte(s) copied successfully.
 Error code : 
     None.
 */
int MemCopy(UINT8_PTR_T pDest, const UINT8_PTR_T pSrc, UINT32_T uiDestLength, UINT32_T uiSrcLength);

/*
 BOOL (* PRINT_CHAR_CALLBACK)(UINT8_T Char, void * Parameter)

 Desc.:
     Print character callback function (user defined function) to passed to UtlVPrintf.
 Params. :
	 Char
	     Character to be displayed or printed.
	 Parameter
	     Pointer to user defined parameter when user defined PRINT_CHAR_CALLBACK is called by UtlVPrintf.
 Return value : 
     None.
 Error code : 
     None.
 */
typedef BOOL (* PRINT_CHAR_CALLBACK)(UINT8_T Char, void * Parameter);


/*
 UINT32_T UtlVPrintf(
					_IN PRINT_CHAR_CALLBACK PrintChar, 
					_IN_OPT void * pPrintCharParam, 
					_IN const char *szFormat, 
					_IN va_list argList
					)

 Desc.:
     Print or display with vector format and with callback.
 Params. :
	 PrintChar
	     Print char callback, please see PRINT_CHAR_CALLBACK above.
	 pPrintCharParam
	     Pointer to user defined parameter to passed to PrintChar when called by UtlVPrintf.
	 szFormat
         Formated string with prefix % (percent) sign. This is very limited format and currently supported format:
         d   = to format or display an integer variable in number format e.g., 2345.
         D   = same as d.
         c   = to format or display a byte variable in single character. Note that character being displayed depends 
               on terminal type, for example ASCI terminal will display character in ASCI.
         s   = to format or display a string (C string or null terminated string) variable. Note that character 
               being displayed depends on teriminal type.
         x   = to format or display an integer variable in haxadecimal format with lower case letter with optional 
               number of digit (1 to 8 digit) before x format, for example %2x and a variable represent by x is 15 
               then will be formated or displayed 0f
         X   = same as x but with capital letter e.g., FE34FF
     argList
	     List of vector argument, must be initialized with va_start() builtin compiler function or a macro.
 Return value : 
     Total number of character(s) that has successfully displayed or printed.
 Error code : 
     None.
 */
UINT32_T UtlVPrintf(
					_IN PRINT_CHAR_CALLBACK PrintChar, 
					_IN_OPT void * pPrintCharParam, 
					_IN const char *szFormat, 
					_IN va_list argList
					);

/*
 UINT32_T DbgPrintf(const char *szFormat, ...)

 Desc.:
     Print format to debug ouput.
 Params. :
	 szFormat
	     Same as UtlVPrintf szFormat parameter.
 Return value : 
     Total number of character(s) that has successfully displayed or printed.
 Error code : 
     None.
 */
UINT32_T DbgPrintf(const char *szFormat, ...);

#ifdef _DEBUG
# define DBG_PRINTF(szFmt, ...)				DbgPrintf(szFmt, __VA_ARGS__)
#else
# define DBG_PRINTF(szFmt, ...)
#endif

/*
 UINT32_T Printf(const char *szFormat, ...)

 Desc.:
     Print format to USB Serial output.
 Params. :
	 szFormat
	     Same as UtlVPrintf szFormat parameter.
 Return value : 
     Total number of character(s) that has successfully displayed or printed.
 Error code : 
     None.
 */
UINT32_T Printf(const char *szFormat, ...);

typedef struct _STREAM_PRINTF_PARAMS {
	UINT8_PTR_T			Buffer;
	UINT32_T			BufferLength;
	UINT32_T			Counter;
}__attribute__((packed)) STREAM_PRINTF_PARAMS, *PSTREAM_PRINTF_PARAMS;

/*
 UINT32_T StreamPrintf(UINT8_PTR_T pBuffer, UINT32_T uBufferLength, const char * szFormat, ...)

 Desc.:
     Format a string to a stream of a buffer with specified length.
 Params. :
	 pBuffer
	     Buffer to receive formatted string (maybe truncated).
	 uBufferLength
	     The length of the Buffer parameter above.
 	 szFormat
	     Same as UtlVPrintf szFormat parameter.
 Return value : 
     Total number of character(s) that has successfully formatted.
 Error code : 
     None.
 */
UINT32_T StreamPrintf(UINT8_PTR_T pBuffer, UINT32_T uBufferLength, const char * szFormat, ...);

#ifdef __cplusplus
}
#endif

#endif  // End of NANOOS_API_H
