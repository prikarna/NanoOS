/*
 * File    : ThreadDef.h
 * Remark  : Contain thread definition, used by thread.c and NanoOS API
 *
 */

#ifndef THREADDEF_H
#define THREADDEF_H

#define THREAD_STATUS__EXIT					0
#define THREAD_STATUS__TERMINATED			1
#define THREAD_STATUS__ACTIVE				2
#define THREAD_STATUS__SUSPENDED			3
#define THREAD_STATUS__SLEEP				4

/*
 int (* THREAD_ENTRY_TYPE)(void * pParm)
 
 Desc.:
     Thread entry point function signature.
 Params. :
     pParm
	     CreateThread() caller allocation parameter to passed to thread entry function when this entry is execute.
		 This parameter is defined by application.
 Return value : 
     int value (application defined return value).
 Error code : 
     None.
*/
typedef int (* THREAD_ENTRY_TYPE)(void * pParm);

#define THREAD__MIN_MSEC_SLEEP			10
#define THREAD__INFINITE_WAIT			0x7FFFFFFF

#define THREAD_CONTROL__PRIVILEGED		0x0
#define THREAD_CONTROL__UNPRIVILEGED	0x1

#define THREAD_WAIT_OBJ__EVENT			0
#define THREAD_WAIT_OBJ__THREAD			1

#define TERM__STACK_REACH_ITS_LIMIT_FAULT			0x80000000
#define TERM__USER_REQUEST							0x40000000

/*
 void (* TERMINATION_HANDLER_TYPE)(UINT32_T uiReason)
 
 Desc.:
     Thread termination handler signature. This handler is called when thread set its termination handler and encounter 
	 fault(s) or being terminated by other thread. Application can not call SetTerminationHandler() in this function.  
	 The termination handler is executed in limited amount of time, approximately after 2 seconds the internal NanoOS
	 timer terminate this handler. If application require more time, it can create new thread in this handler and do 
	 whatever it needs to do there.
 Params. :
     uiReason
	     Reason of being terminated, can be one or more TERM__XXX values defined in NanoOSApi.h and in this header.
 Return value : 
     None.
 Error code : 
     None.
*/
typedef void (* TERMINATION_HANDLER_TYPE)(UINT32_T uiReason);

#define PROBE_STATUS__ERROR				0
#define PROBE_STATUS__ORIGIN			1
#define PROBE_STATUS__FAULT				2
#define PROBE_STATUS__RETURN			3
#define PROBE_STATUS__NO_RETURN			4

#endif  // End of THREADDEF_H
