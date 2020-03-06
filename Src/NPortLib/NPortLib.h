#ifndef NPORTLIB_H
#define NPORTLIB_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (__stdcall * NPL_CONNECTION_CHANGE_CALLBACK)(unsigned char fIsConnected);
typedef void (__stdcall * NPL_DATA_RECEIVED_CALLBACK)(unsigned char * pData, int iDataLength);
typedef void (__stdcall * NPL_ERROR_CALLBACK)(const char * szErrorMessage);

#ifdef NPORTLIB_EXPORTS
# define NPORTLIB		__declspec(dllexport)
#else
# define NPORTLIB
#endif

NPORTLIB
int NPL_Open(
		 NPL_CONNECTION_CHANGE_CALLBACK ConnectionChangeCallback,
		 NPL_DATA_RECEIVED_CALLBACK DataReceivedCallback,
		 NPL_ERROR_CALLBACK ErrorCallback
		 );

NPORTLIB void NPL_Close();

NPORTLIB
int NPL_Write(unsigned char * pBuffer, int iBufferLength);

NPORTLIB
int NPL_Read(unsigned char * pBuffer, int iBufferLength, int * piRead);

#ifdef __cplusplus
}
#endif

#endif  // End of NPORTLIB_H
