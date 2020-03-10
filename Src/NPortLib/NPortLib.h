/*
 * File    : NPortLib.h
 * Remark  : NPortLib function definitions and callbacks.
 *           Provide access to NanoOS port from several languages easier. C and C++ languages
 *           can use this header.
 *           In this header prefix NPL_ mean NanoOS Port Library.
 *
 */

#ifndef NPORTLIB_H
#define NPORTLIB_H

#ifdef __cplusplus
extern "C" {
#endif

/*
	Def.    : Pointer to function which will be called when NanoOS port connection changed. When this callback
			  is called and fIsConnected parameter is 1 this mean NanoOS port is ready for it's operations such
			  as read, write and install.
	Params. : 
		fIsConnected
			1 if NanoOS port is connected and opened or 0 if not connected or closed.
*/
typedef void (__stdcall * NPL_CONNECTION_CHANGE_CALLBACK)(unsigned char fIsConnected);

/*
	Def.    : Pointer to function which will be called when data has successfully read from NanoOS port.
	Params. : 
		pData
			Pointer to byte (unsigned char) of data.
		iDataLength
			Data length. Max. data length of NanoOS port is 64 bytes.
*/
typedef void (__stdcall * NPL_DATA_RECEIVED_CALLBACK)(unsigned char * pData, int iDataLength);

/*
	Def.    : Pointer to function which will be called when an error occured from NanoOS port.
	Params. : 
		szErrorMessage
			Error message in NULL terminated string.
		ulErrorCode
			Error code (Windows API error code).
*/
typedef void (__stdcall * NPL_ERROR_CALLBACK)(const char * szErrorMessage, unsigned long ulErrorCode);

/*
	Def.    : Pointer to function which will be called when an error occured from NanoOS port.
	Params. : 
		iPercentProgress
			Indicate percent (%) of installing progress. (100 indicate complete and success operation)
*/
typedef void (__stdcall * NPL_INSTALLING_CALLBACK)(int iPercentProgress);

#ifdef NPORTLIB_EXPORTS
# define NPORTLIB		__declspec(dllexport)
#else
# define NPORTLIB
#endif

/*
	Funct.	: NPL_Open
	Desc.   : Open NanoOS Port.
	Params. :
		ConnectionChangeCallback
			A callback or pointer to function with the type of NPL_CONNECTION_CHANGE_CALLBACK. This is 
			optional parameter or can be NULL.
		DataReceivedCallback
			A callback or pointer to function with the type of NPL_DATA_RECEIVED_CALLBACK. This parameter
			can be NULL. If this parameter is NULL, then the library will no generate an event when it 
			receive a data and application can use NPL_Read(). If this parameter is non NULL then NPL_Read() 
			will always return 0 (or fail).
		ErrorCallback
			A callback or pointer to function with the type of NPL_ERROR_CALLBACK. This parameter can be NULL.
	Return  : 1 if success otherwise 0.
*/
NPORTLIB
int NPL_Open(
		 NPL_CONNECTION_CHANGE_CALLBACK ConnectionChangeCallback,
		 NPL_DATA_RECEIVED_CALLBACK DataReceivedCallback,
		 NPL_ERROR_CALLBACK ErrorCallback
		 );

/*
	Funct.	: NPL_Close
	Desc.   : Close NanoOS Port.
	Params. : None.
	Return  : None.
*/
NPORTLIB void NPL_Close();

/*
	Funct.	: NPL_Write
	Desc.   : Write data to NanoOS port with specified length.
	Params. :
		pBuffer
			Pointer to byte data to be writen to NanoOS port. 
		iBufferLength
			The length or size of pBuffer above in bytes. Max. data length or size is 64 bytes.
	Return  : 1 if success otherwise 0.
*/
NPORTLIB
int NPL_Write(unsigned char * pBuffer, int iBufferLength);

/*
	Funct.	: NPL_Read
	Desc.   : 
		Read data from NanoOS port with specified buffer and it's length. This function will return 0 if 
		parameter 'DataReceivedCallback' callback in the function NPL_Open() above is non NULL.
	Params. :
		pBuffer
			Pointer to byte buffer data to receive data. 
		iBufferLength
			The length or size of pBuffer above in bytes.
		piRead
			Pointer to int that will receive number of bytes data that has been successfully received. This 
			parameter is optional or can be NULL.
	Return  : 1 if success otherwise 0.
*/
NPORTLIB
int NPL_Read(unsigned char * pBuffer, int iBufferLength, int * piRead);

/*
	Funct.	: NPL_IsOpen
	Desc.   : Tell NanoOS port status (opened or closed).
	Params. : None.
	Return  : 1 if NanoOS port is connected and opened or otherwise 0.
*/
NPORTLIB
int NPL_IsOpen();

/*
	Funct.	: NPL_GetErrorCode
	Desc.   : Return Windows API last error code.
	Params. : None.
	Return  : (Windows API last error code).
*/
NPORTLIB
unsigned long NPL_GetErrorCode();

/*
	Funct.	: NPL_InstallNApplication
	Desc.   : Install NanoOS application. This is asynchronuos function.
	Params. :
		szApplicationFile
			Null terminated string of NanoOS application file. This is required.
		InstallingCallback
			A callback or pointer to function with the type of NPL_INSTALLING_CALLBACK. Called when 
			installing Nano OS Application. This is optional parameter or can be NULL.
	Return  : 1 if success otherwise 0.
*/
NPORTLIB
int NPL_InstallNApplication(
		const char * szApplicationFile,
		NPL_INSTALLING_CALLBACK InstallingCallback
		);

/*
	Funct.	: NPL_IsInstalling
	Desc.   : To tell whether the NanoOS port is still installing NanooS application.
	Params. : None.
	Return  : 1 if NanoOS port is still installing otherwise 0.
*/
NPORTLIB
int NPL_IsInstalling();

/*
	Funct.	: NPL_CancelInstall
	Desc.   : To cancel current NanoOS application installation.
	Params. : None.
	Return  : 1 if success otherwise 0.
*/
NPORTLIB
int NPL_CancelInstall();

#ifdef __cplusplus
}
#endif

#endif  // End of NPORTLIB_H

#ifdef __cplusplus

#endif  // End of __cplusplus
