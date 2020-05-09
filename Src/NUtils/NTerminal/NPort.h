/*
 * File    : NPort.h
 * Remark  : NPort class definition.
 *           To access Serial (COM) port on PC.
 *
 */

#pragma once
#include "TChar.h"
#include "Windows.h"
#include "StrSafe.h"
#include "Dbt.h"

#include "Debug.h"
#include "DkObject.h"

#define MAX_NPORT		4

class NPort
{
public:
	struct ReadDataFormat
	{
		LPVOID			RawData;
		unsigned int	DataLength;
	};

	enum DTRControl
	{
		DTRControl_Disable = 0,
		DTRControl_Enable,
		DTRControl_Handshake
	};

	enum RTSControl 
	{
		RTSControl_Disable = 0,
		RTSControl_Enable,
		RTSControl_Handshake,
		RTSControl_Toggle
	};

	enum Parity
	{
		Parity_None = 0,
		Parity_Odd,
		Parity_Event,
		Parity_Mark,
		Parity_Space
	};

	enum ByteSize
	{
		ByteSize_5 = 5,
		ByteSize_6,
		ByteSize_7,
		ByteSize_8
	};

	enum StopBits
	{
		StopBits_1 = 0,
		StopBits_1Dot5,
		StopBits_2
	};

	enum PurgeOption
	{
		PurgeOption_RxAbort,
		PurgeOption_RxClear,
		PurgeOption_TxAbort,
		PurgeOption_TxClear,
		PurgeOption_RxTxAbort,
		PurgeOption_RxTxClear,
		PurgeOption_All
	};

private:
	HANDLE				m_hDev;
	HANDLE				m_hReadThread;
	TCHAR				m_szErrorBuffer[2048];
	TCHAR				m_szGenStrBuff[1024];
	TCHAR				m_szDevName[512];
	OVERLAPPED			m_WrOvr;
	OVERLAPPED			m_RdOvr;
	volatile bool		m_bStopRead;
	bool				m_bReadEvent;
	HGLOBAL				m_hRdEvtMem;
	LPVOID				m_pRdMem;

	unsigned int		m_uiInQueueSize;			// in bytes
	unsigned int		m_uiOutQueueSize;			// in bytes
	unsigned int		m_uiReadIntervalTimeOut;	// in milliseconds
	bool				m_fNullStripping;			// Enable null stripping

	const TCHAR *		m_szDetWndClassName;
	HWND				m_hDetWnd;
	bool				m_fEnableAutoDet;
	HDEVNOTIFY			m_hNotif;
	HANDLE				m_hDetTh;
	unsigned int		m_DetBaudRate;
	NPort::ByteSize		m_DetByteSize;
	NPort::Parity		m_DetParity;
	NPort::StopBits		m_DetStopBits;

	volatile bool		m_fFormatedReadData;

	CRITICAL_SECTION	m_CritSec;

	DWORD				m_dwLastError;

	static NPort *		m_PortList[MAX_NPORT];

	static DWORD CALLBACK	_Read(LPVOID pDat);

	static LRESULT CALLBACK	_DetectProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm);
	bool					_HandleDetectProc(UINT uMsg, WPARAM wParm, LPARAM lParm);

	static DWORD CALLBACK _DoDetection(LPVOID pDat);

	void _FormatString(const TCHAR *szFormat, ...);
	void _HandleError(const TCHAR *szErrorMessage, DWORD dwError);

	// Property functions
	HANDLE			_GetHandle();
	unsigned int	_GetInputQueueSize();
	void			_SetInputQueueSize(unsigned int uiQueueSize);
	unsigned int	_GetOutputQueueSize();
	void			_SetOutputQueueSize(unsigned int uiQueueSize);
	unsigned int	_GetTimeOut();
	void			_SetTimeOut(unsigned int uiTimeOut);
	bool			_GetIsOpen();
	void			_SetNullStripping(bool fEnable);
	bool			_GetNullStripping();
	const TCHAR *	_GetDeviceName();
	bool			_GetReadEvent();
	void			_SetReadEvent(bool fEnable);
	bool			_GetFormatReadData();
	void			_SetFormatReadData(bool fEnable);
	DWORD			_GetLastError();

public:
	NPort(void);
	~NPort(void);

	/* Read data size in bytes, must set before open the device */
	unsigned int		ReadDataSize;

	/* Disable OnError event when set to true */
	bool				SuppressError;

/*
	Event			: OnDataReceived
	Desc.			: Triggered when data received. ReadEvent property must be set to true. (default: true).
	Handler param.	: multibyte string or byte stream data.
*/
	Event<NPort, char *>					OnDataReceived;

/*
	Event			: OnError
	Desc.			: Triggered when an error ocuured.
	Handler param.	: string of error message.
*/
	Event<NPort, const TCHAR *>			OnError;

/*
	Event			: OnEnumCOM
	Desc.			: Triggered EnumCom function found a serial or COM port.
	Handler param.	: string of serial or COM name.
*/
	Event<NPort, const TCHAR *>			OnEnumCOM;

/*
	Event			: OnDeviceChange
	Desc.			: Triggered when auto detection is enabled and when device connect or disconnect.
	Handler param.	: true if device is connected or otherwise false.
*/
	Event<NPort, bool>						OnDeviceChange;

/*
	Event			: OnFormatedReadData
	Desc.			: Triggered when FormatedReadData is set to true.
	Handler param.	: Pointer to NPort::ReadDataFormat.
*/
	Event<NPort, NPort::ReadDataFormat *>	OnFormatedReadData;

/*
	Prop.		: Handle.
	Var. type	: HANDLE.
	Direction	: get.
	Desc.		: Handle to currently opened device or INVALID_HANDLE_VALUE if not opened.
*/
	Property<NPort, HANDLE>			Handle;

/*
	Prop.		: InputQueueSize.
	Var. type	: unsigned int.
	Direction	: set and get.
	Desc.		: Input queue size.
*/
	Property<NPort, unsigned int>	InputQueueSize;

/*
	Prop.		: OutputQueueSize.
	Var. type	: unsigned int.
	Direction	: set and get.
	Desc.		: Output queue size.
*/
	Property<NPort, unsigned int>	OutputQueueSize;

/*
	Prop.		: TimeOut.
	Var. type	: unsigned int.
	Direction	: set and get.
	Desc.		: Read interval time out.
*/
	Property<NPort, unsigned int>	TimeOut;

/*
	Prop.		: IsOpen.
	Var. type	: bool.
	Direction	: get.
	Desc.		: Status of device, true if opened otherwise false.
*/
	Property<NPort, bool>			IsOpen;

/*
	Prop.		: NullStripping.
	Var. type	: bool.
	Direction	: set and get.
	Desc.		: Enable or disable null stripping on setup a device.
*/
	Property<NPort, bool>			NullStripping;

/*
	Prop.		: DeviceName.
	Var. type	: const TCHAR *. (string)
	Direction	: get.
	Desc.		: Get serial or COM name of currently opened device.
*/
	Property<NPort, const TCHAR *>	DeviceName;

/*
	Prop.		: ReadEvent.
	Var. type	: bool.
	Direction	: set and get.
	Desc.		: Enable or disable OnDataReceived or OnFormatedReadData event depend on FormatedReadData property.
*/
	Property<NPort, bool>			ReadEvent;

/*
	Prop.		: FormatedReadData.
	Var. type	: bool.
	Direction	: set and get.
	Desc.		: Enable or disable OnFormatedReadData event rather than OnDataReceived.
*/
	Property<NPort, bool>			FormatedReadData;

/*
	Prop.		: LastError.
	Var. type	: DWORD (unsigned long).
	Direction	: get.
	Desc.		: Return error code when an error occured from this class instance. This is
				  Windows API error code.
*/
	Property<NPort, DWORD>			LastError;

/*
	Funct.	: Open
	Desc.   : Open a Serial (COM) port.
	Params. :
		szComName
			Serial (COM) port name.
		uiBaudRate
			Baudrate of serial port.
		eByteSize
			Byte size of serial port, one of NPort::ByteSize enumeration.
		eParity
			Parity type, one of NPort::Parity enumeration.
		eStopBits
			Stop bits, one of NPort::StopBits enumeration.
		eDTRControl
			DTR control, one of NPort::DTRControl enumeration.
		eRTSControl
			RTS control, one of NPort::RTSControl enumeration.
	Return  : true if success otherwise false.
*/
	bool Open(
			const TCHAR *szComName, 
			unsigned int uiBaudRate,
			NPort::ByteSize eByteSize, 
			NPort::Parity eParity, 
			NPort::StopBits eStopBits,
			NPort::DTRControl eDTRControl,
			NPort::RTSControl eRTSControl
			);

/*
	Funct.	: Close
	Desc.   : Close Serial (COM) port if opened.
	Params. : None.
	Return  : true if success otherwise false.
*/
	bool Close();

/*
	Funct.	: Write
	Desc.   : Write a byte to serial port and trigger OnError when failed.
	Params. : 
		ucDat
			Byte data to write to serial port.
	Return  : true if success or otherwise false.
*/
	bool Write(unsigned char ucDat);

/*
	Funct.	: Write (Overload 1)
	Desc.   : Write byte stream with specified length to serial port and trigger OnError when failed.
	Params. : 
		pDat
			Pointer to byte stream data.
		uiDataSize
			Byte stream data size of length.
	Return  : true if success or otherwise false.
*/
	bool Write(const unsigned char * pDat, unsigned int uiDataSize);

/*
	Funct.	: Read
	Desc.   : Read a byte data from serial port.
	Params. : 
		pBuffer
			Pointer to byte buffer to receive byte data.
	Return  : true if success or otherwise false.
*/
	bool Read(unsigned char * pBuffer);

/*
	Funct.	: Read
	Desc.   : Read serial port for specified buffer and length and trigger OnError if failed.
	Params. : 
		pBuffer
			Pointer to byte buffer to receive data when read operation success.
		uiBufferSize
			Byte buffer size or length in bytes.
		uiRead
			Reference to unsigned int variable that will receive number of byte that has
			been successfully read.
	Return  : true if success or otherwise false.
*/
	bool Read(unsigned char * pBuffer, unsigned int uiBufferSize, unsigned int &uiRead);

/*
	Funct.	: Purge
	Desc.   : 
		Discard input and output buffer and terminate read write pending operation.
	Params. : 
		PurgeOption
			Can be one of the following:
				PurgeOption_RxAbort		: Termnated all outstanding overlapped read operation.
				PurgeOption_RxClear		: Clear input buffer.
				PurgeOption_TxAbort		: Terminate all outstanding overlapped write operation.
				PurgeOption_TxClear		: Clear output buffer.
				PurgeOption_RxTxAbort	: Combination of PurgeOption_RxAbort and PurgeOption_TxAbort.
				PurgeOption_RxTxClear	: Combination of PurgeOption_RxClear and PurgeOption_TxClear.
				PurgeOption_All			: All of the above combinations.
	Return  : true if success or otherwise false.
*/
	bool Purge(enum NPort::PurgeOption PurgeOption);

/*
	Funct.	: EnumComm
	Desc.   : 
		Enumerate serial or COM port available on the system and trigger OnEnumCOM when found 
		a serial or COM port. Use COM name, e.g. COM1, COM2 and so on, rather than device name.
	Params. : None.
	Return  : None.
*/
	void EnumComm();

/*
	Funct.	: EnableAutoDetection
	Desc.   : 
		Enable auto detection mode. When connected this class instance will open automatically the specified serial 
		port in m_szDevName buffer and when disconnected this mechanism will close it automatically. This only 
		usefull when serial (COM) port device is a dynamic device like USB Serial converter.
	Params. : 
		szComName
			Serial (COM) port name to be detected. This name should be something like : 
			\\\\?\\USB#Vid_XXX&Pid_YYYY#<SerialNumber>#{a5dcbf10-6530-11d2-901f-00c04fb951ed} where XXXX is VID,
			YYYY is PID and <SerialNumber> is serial number of USB device. These three parameters can be found in 
			the device manager or WDK's USBView program.
		uiBaudRate
			Serial (COM) port baud rate.
		eByteSize
			Byte data size, one of NPort::ByteSize enumeration.
		eParity
			Parity type, one of NPort::Parity enumeration.
		eStopBits
			Stop bits, one of NPort::StopBits enumeration.
	Return  : true if success or otherwise false.
*/
	bool EnableAutoDetection(
							const TCHAR *szComName, 
							unsigned int uiBaudRate,
							NPort::ByteSize eByteSize, 
							NPort::Parity eParity, 
							NPort::StopBits eStopBits
							);

/*
	Funct.	: DisableAutoDetection
	Desc.   : Disable the auto detection.
	Params. : None.
	Return  : None.
*/
	void DisableAutoDetection();

	bool ReadAsynch(unsigned char * pBuffer, unsigned int uiBufferLength, unsigned int & uiReadLen);
	bool WriteAsynch(unsigned char * pData, unsigned int uiDataLength);
	bool GetAsynchReadResult(unsigned char * pBuffer, unsigned int uiBufferLength, unsigned int & uiReadLen);
	bool GetAsynchWriteResult(unsigned char * pData, unsigned int uiDataLength);
	bool HasAsynchReadComplete();
	bool Cancel();
};
