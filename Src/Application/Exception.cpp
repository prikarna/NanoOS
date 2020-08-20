/*
 * File   : Exception.cpp
 * Remark : Minimum implementation C++ exception.
 *          This implementation is just 'satisfying the compiler', in this case GCC, 
 *          rather than complying with standard ABI and may not work for untested 
 *          cases. For supported usage or cases, please see Main.cpp in this project.
 *          Per thread basis exception and for now can only be used by one thread.
 *          Can not handle multiple try-catch in one function such as codes below, 
 *          in pseudo code:
 *          ...
 *          void SomeFunc()
 *          {
 *              try {
 *                  DoRiskyTask1();
 *                  DoRiskyTask2();
 *              }
 *              catch (int i) {
 *                  HandlerTaskError();
 *              }
 *              ...
 *              try {
 *                  DoRiskyBussiness();
 *              }
 *              catch (Error e) {
 *                  HandleError();
 *              }
 *          }
 *          ...
 */

#include <typeinfo>
#include "cxxabi.h"
#include "NanoOSApi.h"

// Exported from TypeInfo.cpp
extern std::type_info	_ZTIi;
extern std::type_info	_ZTIPc;
extern std::type_info	_ZTIPKc;
extern std::type_info	_ZTId;
extern std::type_info	_ZTIc;

/*
 * Declare own __class_type_info.
 * To take its first function address of __class_type_info vtable
 * then compare it to user first function address of its vtable,
 * please see GetUnwindData() for more.
 */
__cxxabiv1::__class_type_info	ClassTypeInfo("NCTI");

extern "C"
{
/*
 * Define 'landing mark', a mark where exception mechanism decide to
 * branch to the begining of exception handler.
 * Based on assembly codes:
 *
 *    mov        [Reg], r0
 *    mov        [Reg], r1
 *    cmp        [Reg], #[Number]
 *    b{xx}.{y}  #[Offset]
 *
 * This implementation uses this mark to identify the begining
 * of exception handler.
 */
#define LANDING_MARK_1		0x468F4687
#define LANDING_MARK_2		0xD1FF2FFF

#define MAX_EXCEPTION_SIZE	128
#define EPILOGUE_SIZE		8

struct _RAISE_PARAMS {
	BOOL			Result;
	UINT32_T		EpilogueAddr;
	UINT8_T			Data8;
	UINT8_T			Count;
}__attribute__((packed));
typedef struct _RAISE_PARAMS	RAISE_PARAMS, *PRAISE_PARAMS;

/*
 * Simple unwind data structure and this is non-standard.
 */
struct _UNWIND {
	UINT32_T		TID;				// Thread ID.
	UINT32_T		LinkRegister;		// As address to begin unwind.
	void *			TypeInfoPtr;		// Pointer to user type info.
	void *			ExceptionPtr;		// Pointer to user exception object.
	BOOL			HasGxxPersonality;	// Determine that this unwind data has __gxx_peronality_v0().
	UINT8_T			FrameReg;			// Register that compiler uses as base frame.
	int				StackDelta;			// Stack size to unwind.
	UINT16_T		PopRegs;			// Registers that must be pop from stack to restore previous 
										// function frame.
	
	UINT32_T		LandingAddr;		// Address where exceptions handler begin. Note that this is not 
										// __cxa_begin_catch() address. This is the address of where the
										// exception mechanism decide to branch to one or more handler.

	UINT32_T		NumbOfLandingAddr;	// For debugging purpose only.

	UINT8_T			HandlerIndex;		// Handler parameter index to be passed to begin an exceptions handling.
										// This index specify which paramter to be passed to correct handler.

	UINT32_T		NumbOfHandlers;		// For debugging purpose only.
}__attribute__((packed));
typedef struct _UNWIND			UNWIND, *PUNWIND;

struct _USER_TYPE_INFO {
	void *						IfacePtr;	// Actually this an address of the the first virtual function
											// in type info vtable structure, but for the shake of simplicity,
											// based on its usage, let just call this IfacePtr (Interface 
											// Pointer) or pointer to one of __cxxabiv1::xxx vtable.

	char *						Name;		// Name of type of this type info.

	struct _USER_TYPE_INFO *	BasePtr;	// Point to base type info. Valid when vtable pointed by IfacePtr
											// is __si_class_type_info or __vmi_class_type_info vtable.

}__attribute__((packed));
typedef struct _USER_TYPE_INFO	USER_TYPE_INFO, *PUSER_TYPE_INFO;

extern unsigned int	__start_exidx, __end_exidx;
extern unsigned int __start_global_object_init;

static UNWIND			sUnwind;
static RAISE_PARAMS		sParams;
static UINT8_T			sException[MAX_EXCEPTION_SIZE];		// As exception data storage
static UINT16_T			sEpilogue[EPILOGUE_SIZE];			// As storage of epilogue codes

void __cxa_end_catch();

/*
 * Get landing address, address of where this exception mechanism decide to branch to one or 
 * more exception handler. It is done in two steps: find BL or BLX instruction that call 
 * 'datum function' and, if found, then find codes that match with LANDING_MARK_1 and 
 * LANDING_MARK_2.
 */
BOOL GetLandingAddress(
					   UINT32_T uiStartAddress, 
					   UINT32_T uiEndAddress, 
					   UINT32_T uiIndex, 
					   PUNWIND pUnwind, 
					   BOOL fDumpMode
					   )
{
	UINT16_PTR_T pTmp;
	UINT32_T uiOffset;
	UINT16_PTR_T pAddr;

	UINT32_T uiDatumAddr;
	UINT32_T uiTmp;
	BOOL fFound = FALSE;

	if ((uiStartAddress == 0) || 
		(uiEndAddress == 0) ||
		(!pUnwind))
	{
		return FALSE;
	}

	if (uiStartAddress >= uiEndAddress) return FALSE;

	uiStartAddress				&= ~BITHEX_0;	// Clear bit 0.
	pUnwind->LandingAddr		= 0;
	pUnwind->NumbOfLandingAddr	= 0;			// Reset numb. of landing addr.

	uiDatumAddr		= (UINT32_T) &__cxa_end_catch;		// Use __cxa_end_catch() as datum address
	uiDatumAddr		&= ~BITHEX_0;						// Clear bit 0
	pAddr			= (UINT16_PTR_T) uiStartAddress;

	//DBG_PRINTF("%s: uiStartAddress=0x%X\r\n", __FUNCTION__, uiStartAddress);

	while (pAddr < (UINT16_PTR_T) uiEndAddress)
	{
		/*
		 * Assume compiler uses BL or BLX instruction to generate 
		 * a call to 'datum function' in this case __cxa_end_catch().
		 */
		if ((*pAddr & 0xF000) == 0xF000)
		{
			pTmp = pAddr;
			uiOffset = ((UINT32_T) (*pTmp & 0x07FF)) << 12;
			pTmp++;
			if ((*pTmp & 0xF800) == 0xF800)
			{
				uiOffset = uiOffset + ((*pTmp & 0x07FF) * 2);
				pTmp++;
				uiTmp = (UINT32_T) pTmp;
				uiTmp = ((uiTmp + uiOffset) & 0xFFFF);

				if (uiTmp == uiDatumAddr)
				{
					/*
					 * Find landing address using LANDING_MARK_1 and LANDING_MARK_2
					 * by 'OR'ing them with specified address content.
					 */
					pTmp = pAddr;
					while (pTmp < (UINT16_PTR_T) uiEndAddress)
					{
						uiTmp = *(UINT32_PTR_T) pTmp;
						if ((uiTmp | LANDING_MARK_1) == LANDING_MARK_1) 
						{
							pTmp += 2;
							uiTmp = *(UINT32_PTR_T) pTmp;
							pTmp -= 2;

							if ((uiTmp | LANDING_MARK_2) == LANDING_MARK_2) 
							{
								
								pUnwind->NumbOfLandingAddr++;
								
								//DBG_PRINTF("%s: pTmp=0x%X, *pTmp=0x%X, numb.=%d, req. idx=%d\r\n",
								//	__FUNCTION__, pTmp, *(UINT32_PTR_T) pTmp, pUnwind->NumbOfLandingAddr, uiIndex);
								
								if (uiIndex == (pUnwind->NumbOfLandingAddr - 1)) {
									pUnwind->LandingAddr = (UINT32_T) pTmp;
									fFound = TRUE;
								}
								break;
							}

						} // End of if ((uiTmp | LANDING_MARK_1) == LANDING_MARK_1) 

						pTmp++;

					} // End of while (pTmp < (UINT16_PTR_T) uiEndAddress)

					pAddr = pTmp;

				} // End of if (uiTmp == uiDatumAddr)

			} // End of if ((*pTmp & 0xF800) == 0xF800)
		}

		if (!fDumpMode) {
			if (fFound) break;
		}

		pAddr++;
	}

	if (fDumpMode) {
		return TRUE;
	}

	return fFound;
}

/*
 * Get unwind data, such as base frame register, stack size to unwind. Also,
 * find parameter index to be 'passed on' to begin an exception. This index
 * represent which parameter used by a handler.
 */
BOOL GetUnwindData(UINT32_PTR_T pTableAddress, PUNWIND pUnwind, BOOL fDumpMode)
{
	UINT8_T	uDat, uHdr, uNthShift, uEntries, uCnt;
	BOOL fRes = FALSE;
	PUSER_TYPE_INFO pti, pcti;

	if (!pUnwind) return FALSE;

	if (fDumpMode) {
		if (!pTableAddress) return FALSE;
	} else {
		if ((!pTableAddress) || (!pUnwind->TypeInfoPtr)) return FALSE;
	}

	pUnwind->FrameReg		= 0;
	pUnwind->HandlerIndex	= 0;
	pUnwind->NumbOfHandlers	= 0;
	pUnwind->PopRegs		= 0;
	pUnwind->StackDelta		= 0;

	uHdr = 0;
	uNthShift = 3;
	while (TRUE)
	{
		uDat = (*pTableAddress >> (8 * uNthShift)) & 0xFF;

		// Header
		if ((uNthShift == 3) && (uHdr == 0)) 
		{
			uHdr = uDat;
			if (uHdr == 0x7F) {
				pUnwind->HasGxxPersonality = TRUE;
				pTableAddress++;
				uEntries = (*pTableAddress >> (8 * uNthShift)) & 0xFF;
				uNthShift--;
			} else {
				pUnwind->HasGxxPersonality = FALSE;
				if (uHdr == 0x80) {
					uNthShift--;
				} else {
					uNthShift -= 2;	// Skip the info of additional entries
				}
			}
			continue;
		}

		// Stack delta
		if (((uDat & 0xF0) == 0) ||
			((uDat & 0xF0) == 0x40))
		{
			if (uDat < 0x40) {
				pUnwind->StackDelta += (int) (((uDat & 0x3F) << 2) + 4);
			} else {
				pUnwind->StackDelta -= (int) (((uDat & 0x3F) << 2) - 4);
			}
		}

		// Frame register
		if ((uDat & 0xF0) == 0x90)
		{
			pUnwind->FrameReg = (uDat & 0x0F);
		}

		// Pop registers (form 1)
		if ((uDat & 0xF0) == 0x80)
		{
			pUnwind->PopRegs = ((UINT16_T) (uDat & 0x0F) << 8);
			if (uNthShift > 0) {
				uNthShift--;
			} else {
				pTableAddress++;
				uNthShift = 3;
			}
			uDat = (*pTableAddress >> (8 * uNthShift)) & 0xFF;
			pUnwind->PopRegs |= ((UINT16_T) uDat);
		}

		// Pop register (form 2)
		if ((uDat & 0xF0) == 0xA0)
		{
			uCnt = uDat & 0x7;
			for (UINT8_T u = 0; u <= uCnt; u++) {
				pUnwind->PopRegs |= (((UINT16_T) 0x1) << u);
			}
			if ((uDat & 0x08) == 0x08) {
				pUnwind->PopRegs |= BITHEX_10;
			}
		}

		// Finish
		if (((uDat & 0xF0) == 0xB0) ||
			(uDat == 0))
		{
			break;
		}

		if (uNthShift > 0) {
			uNthShift--;
		} else {
			if (uHdr == 0x80) {
				break;
			} else if ((uHdr == 0x7F) && (uEntries == 0)) {
				break;
			} else {
				pTableAddress++;
				uNthShift = 3;
			}
		}
	}

	if (uHdr != 0x7F) return TRUE;

	/*
	 * !!! THIS MAYBE WRONG !!!
	 */
	/*
	 * Find parameter index in the unwind table based on user type info pointer.
	 */

	pTableAddress++;
	uDat = (*pTableAddress >> 16) & 0xFF;		// Assume this is bytes length of this entry
	uEntries = (*pTableAddress >> 8) & 0xFF;	// Assume this is an indication of handler params

	uDat = uDat / sizeof(UINT32_T);

	pTableAddress += uDat;
	pti = (PUSER_TYPE_INFO) pUnwind->TypeInfoPtr;

	for (UINT8_T u = 0; u < uDat; u++)
	{
		if ((*pTableAddress & 0xFF000000) == 0xFF000000)
		{
			/*
			 * This table address content should be reference or address of one of 
			 * __cxxabiv1::xxx object.
			 */
			pUnwind->NumbOfHandlers++;

			pcti = (PUSER_TYPE_INFO) &ClassTypeInfo;

			//DBG_PRINTF("%s: pti->IfacePtr=0x%X, pcti->IfacePtr=0x%X\r\n", 
			//	__FUNCTION__, pti->IfacePtr, pcti->IfacePtr);

			// Compare user type info iface with our type info iface.
			if (pti->IfacePtr != pcti->IfacePtr) 
			{
				/*
				 * User type info is NOT __cxxabiv1::__class_type_info, which mean user exception object
				 * is inherited from other class, so select its base class type info first if match.
				 */
				if (pti->BasePtr == (PUSER_TYPE_INFO) (((UINT32_T) pTableAddress) + *pTableAddress))
				{
					fRes = TRUE;
					pUnwind->HandlerIndex = pUnwind->NumbOfHandlers;
					if (!fDumpMode) break;
				}

				/*
				 * If no base class type info then select user type info if match.
				 */
				if (pti == (PUSER_TYPE_INFO) (((UINT32_T) pTableAddress) + *pTableAddress))
				{
					fRes = TRUE;
					pUnwind->HandlerIndex = pUnwind->NumbOfHandlers;
					if (!fDumpMode) break;
				}
			}
			else
			{
				/*
				 * User type info is __cxxabiv1::__class_type_info, so select user type info if match.
				 */
				if (pti == (PUSER_TYPE_INFO) (((UINT32_T) pTableAddress) + *pTableAddress))
				{
					pUnwind->HandlerIndex = pUnwind->NumbOfHandlers;
					fRes = TRUE;
					if (!fDumpMode) break;
				}
			}
		}

		if ((*pTableAddress & 0xFF000000) == 0x1F000000)
		{
			/*
			 * This table address content should be reference or address of std::type_info,
			 * in the other word this shoud be reference or address of one of builtin variable 
			 * type info object (_ZTI[xxx]), so select it if match.
			 */
			pUnwind->NumbOfHandlers++;

			if (pti == (PUSER_TYPE_INFO) (((UINT32_T) pTableAddress) + *pTableAddress))
			{
				fRes = TRUE;
				pUnwind->HandlerIndex = pUnwind->NumbOfHandlers;
				if (!fDumpMode) break;
			}
		}

		if (*pTableAddress == 0)
		{
			/*
			 * Most likely this is an indication of 'default handler'.
			 */
			pUnwind->NumbOfHandlers++;
			pUnwind->HandlerIndex = pUnwind->NumbOfHandlers;
			fRes = TRUE;
			if (!fDumpMode) break;
		}

		pTableAddress--;
	}

	if (fDumpMode) {
		if (pUnwind->NumbOfHandlers > 0)
			return TRUE;
	} else {
		if (fRes)
			return TRUE;
	}

	/*
	 * If we can't find handler index, we search again if there is params indication.
	 */
	if (uEntries != 0xFF) {
		pUnwind->NumbOfHandlers = 0;
		for (uCnt = 0; uCnt < 255; uCnt++)
		{
			if (*pTableAddress == 0) break;

			if (((*pTableAddress & 0xFF000000) == 0xFF000000) ||
				((*pTableAddress & 0xFF000000) == 0x1F000000))
			{
				sUnwind.NumbOfHandlers++;
				if (pti == (void*) (((UINT32_T) pTableAddress) + *pTableAddress))
				{
					fRes = TRUE;
					pUnwind->HandlerIndex = pUnwind->NumbOfHandlers;
					if (!fDumpMode) break;
				}
			}
			pTableAddress++;
		}
	}

	if (fDumpMode) 
		return TRUE;

	return fRes;
}

/*
 * To find matching unwind data in unwind table based on an address in a function.
 */
BOOL FindUnwindData(UINT32_T uiAddress, PUNWIND pUnwind)
{
	UINT32_PTR_T	pIdxLow;
	UINT32_PTR_T	pIdxHigh;
	UINT8_PTR_T		pTable;
	BOOL			fRes = FALSE;
	UINT32_T		uiAddrLow, uiAddrHigh;

	if (!pUnwind) return FALSE;

	if (uiAddress == 0) return FALSE;

	pIdxLow = &__start_exidx;
	while (pIdxLow < &__end_exidx)
	{
		pIdxHigh = pIdxLow;
		pIdxHigh += 2;

		if (pIdxHigh > &__end_exidx) break;

		uiAddrLow = (UINT32_T) pIdxLow - ((~(*pIdxLow - 1)) & 0xFFFF);
		uiAddrHigh = (UINT32_T) pIdxHigh - ((~(*pIdxHigh - 1)) & 0xFFFF);;

		if ((uiAddress >= uiAddrLow) &&
			(uiAddress < uiAddrHigh))
		{
			pIdxLow++;
			if (*pIdxLow > 0x1) {
				if ((*pIdxLow & BITHEX_31) == BITHEX_31)
				{
					fRes = GetUnwindData(pIdxLow, pUnwind, FALSE);
					if (fRes) {
						GetLandingAddress(uiAddress, uiAddrHigh, 0, pUnwind, FALSE);
					}
				}
				else if ((*pIdxLow & 0x70000000) == 0x70000000)
				{
					pTable = ((UINT8_PTR_T) pIdxLow) - ((~(*pIdxLow - 1)) & 0xFFFF);
					fRes = GetUnwindData((UINT32_PTR_T) pTable, pUnwind, FALSE);
					if (fRes) {
						GetLandingAddress(uiAddress, uiAddrHigh, 0, pUnwind, FALSE);
					}
				}
				else
				{
					// Do nothing, let false result
				}
			}
			break;
		}

		pIdxLow += 2;
	}

	return fRes;
}

extern int main(int, char*[]);
void DbgDumpUnwindData();

/*
 * Raise an exception
 */
void RaiseException()
{
	/*
	 * Since this function will change the stack (for unwinding operation), 
	 * we can't use local variables here. And this function can't take one 
	 * or more parameter. This because compiler uses stack to allocate storage 
	 * for local variable(s) and function parameter(s).
	 */

	sUnwind.LinkRegister = (UINT32_T) &RaiseException;	// First, use this function address to unwind the stack
	sParams.Result = TRUE;

	while (sParams.Result)
	{
		if (sUnwind.LinkRegister < (UINT32_T) &__start_global_object_init) {
			DBG_PRINTF("%s: Unwinding has reach the minimum address!\r\n", __FUNCTION__);
			Printf("(Thread %d exits abnormally)\r\n", sUnwind.TID);	// Notify user program
			ExitThread();
		}

		sParams.Result = FindUnwindData(sUnwind.LinkRegister, &sUnwind);
		if (!sParams.Result) {
			DBG_PRINTF("%s: Can't unwind!\r\n", __FUNCTION__);
			DbgDumpUnwindData();
			Printf("(Thread %d exits abnormally)\r\n", sUnwind.TID);	// Notify user program
			ExitThread();
		}

		/*
		 * If conditions are meet, start execute handler. If not then continue unwind the stack.
		 */
		if ((sUnwind.HasGxxPersonality) &&
			(sUnwind.LandingAddr != 0) &&
			(sUnwind.HandlerIndex != 0))
		{
			DBG_PRINTF("%s: Branch to first landing addr. 0x%4X, index=%d\r\n", 
				__FUNCTION__, sUnwind.LandingAddr, sUnwind.HandlerIndex);

			// Branch to the begining of exception handler.
			sUnwind.LandingAddr |= BITHEX_0;	// Ensure LSB is 1.
			asm(
				"mov.w r8, %0;"
				"mov.w r6, %1;"
				"mov.w r5, %2;"
				"mov.w r1, r6;"
				"mov.w r0, r5;"
				"bx r8;"
				: : "r" (sUnwind.LandingAddr), "r" (sUnwind.HandlerIndex), "r" (sUnwind.ExceptionPtr)
				);

		} else {

			DBG_PRINTF("%s: Unwind for addr. 0x%4X\r\n", __FUNCTION__, sUnwind.LinkRegister);

			if (sUnwind.StackDelta < 0)
			{
				DBG_PRINTF("%s: Can't handle negative stack delta!\r\n", __FUNCTION__);
				Printf("(Thread %d exits abnormally)\r\n", sUnwind.TID);
				ExitThread();
			}
			if ((sUnwind.FrameReg > 12) ||
				(sUnwind.FrameReg == 8) ||
				(sUnwind.FrameReg == 9))
			{
				DBG_PRINTF("%s: Invalid frame register!\r\n", __FUNCTION__);
				Printf("(Thread %d exits abnormally)\r\n", sUnwind.TID);
				ExitThread();
			}

			/*
			 * Construct and execute an epilogue code in SRAM.
			 */
			// adds <Frame reg.>, #Stack delta
			for (sParams.Count = 0; sParams.Count < 4; sParams.Count++)
			{
				sParams.Data8 = (sUnwind.StackDelta >> (8 * sParams.Count)) & 0xFF;
				if (sParams.Data8 > 0) {
					sEpilogue[sParams.Count] = 
								0x3000 | 
								((((UINT16_T) sUnwind.FrameReg) & 0x7) << 8) | 
								((UINT16_T) sParams.Data8);
				} else {
					sEpilogue[sParams.Count] = 0xbf00;
				}
			}

			// mov sp, <Frame reg.>
			sEpilogue[4] = 0x4600 |
						 (( ((UINT16_T) sUnwind.FrameReg & 0x0F) ) << 3) |
						 0x85;

			// ldmia.w sp!, <Registers>
			sEpilogue[5] = 0xe8bd;
			sEpilogue[6] = (sUnwind.PopRegs << 4);

			// bx r8
			sEpilogue[7] = 0x4700 | (((UINT16_T) 0x0008) << 3);

			asm(
				"nop;"
				"nop;"
				"mov r8, pc;"
				"add r8, #9;"		// Set 'branch back' address from SRAM epilogue code.
				"mov.w r9, %1;"
				"bx r9;"			// Execute epilogue
				"nop;"
				"nop;"
				"mov %0, lr;"
				: "=r" (sUnwind.LinkRegister) : "r" (sParams.EpilogueAddr)
				);
		}
	}

	DBG_PRINTF("%s: Error!\r\n", __FUNCTION__);
	Printf("(Thread %d exits abnormally)\r\n", sUnwind.TID);
	ExitThread();
}

UINT32_T __aeabi_unwind_cpp_pr0(UINT32_T State, void* pUCB, void* pContext)
{
	DBG_PRINTF("%s\r\n", __FUNCTION__);
	return 0;
}

UINT32_T __aeabi_unwind_cpp_pr1(UINT32_T State, void* pUCB, void* pContext)
{
	DBG_PRINTF("%s\r\n", __FUNCTION__);
	return 0;
}

void __gxx_personality_v0()
{
	DBG_PRINTF("%s\r\n", __FUNCTION__);
}

void __cxa_end_cleanup()
{
	DBG_PRINTF("%s\r\n", __FUNCTION__);
}

void* __cxa_allocate_exception(unsigned int uiSize)
{
	DBG_PRINTF("%s(%d)\r\n", __FUNCTION__, uiSize);
	
	if (uiSize > MAX_EXCEPTION_SIZE) {
		DBG_PRINTF("%s: Requested size is too large!\r\n", __FUNCTION__);
		Printf("(Thread %d exits abnormally)\r\n", sUnwind.TID);
		ExitThread();
	}

	MemSet(&(sException[0]), 0, MAX_EXCEPTION_SIZE);
	
	return (void*) &(sException[0]);
}

void DbgDumpUnwindTable();

void __cxa_throw(void * pException, std::type_info * pTypeInfo, void *p)
{
	DBG_PRINTF("%s(0x%X, 0x%X, 0x%X)\r\n", __FUNCTION__, pException, pTypeInfo, p);

	sUnwind.ExceptionPtr	= pException;
	sUnwind.TypeInfoPtr		= pTypeInfo;
	
	GetCurrentThreadId(&sUnwind.TID);

	sParams.EpilogueAddr	= (UINT32_T) &sEpilogue[0];
	sParams.EpilogueAddr	|= BITHEX_0;	// Ensure LSB is 1

	RaiseException();
	
	//DbgDumpUnwindTable();
	//ExitThread();
}

void __cxa_rethrow()
{
	DBG_PRINTF("%s\r\n", __FUNCTION__);
	RaiseException();
}

void * __cxa_begin_catch(void *p)
{
	void* pRet;

	DBG_PRINTF("%s\r\n", __FUNCTION__);

	if (sUnwind.TypeInfoPtr == (void*) &_ZTIi)
	{
		pRet = p;
	}
	/*
	 * If user type info indicate a pointer, the content of exception is a pointer so
	 * we return this pointer instead.
	 */
	else if ((sUnwind.TypeInfoPtr == (void*) &_ZTIPKc) || (sUnwind.TypeInfoPtr == (void*) &_ZTIPc))
	{
		pRet = *(void**)sUnwind.ExceptionPtr;
	}
	else
	{
		pRet = p;
	}
	return pRet;
}

void __cxa_end_catch()
{
	DBG_PRINTF("%s\r\n", __FUNCTION__);
}

void __cxa_free_exception(void *p)
{
	DBG_PRINTF("%s(0x%X)\r\n", __FUNCTION__, p);
}

void* __cxa_get_exception_ptr(void* p)
{
	DBG_PRINTF("%s(0x%X)\r\n", __FUNCTION__, p);
	return (void *) &(sException[0]);
}

void __cxa_call_unexpected(void* p)
{
	DBG_PRINTF("%s(0x%X)\r\n", __FUNCTION__, p);
	ExitThread();
	while(1);	// Just to satisfy the compiler
}

void __cxa_call_terminate(void* p)
{
	DBG_PRINTF("%s(0x%X)\r\n", __FUNCTION__, p);
	ExitThread();
}

/* 
 * To dump and interpret the contents of unwind table.
 */
void DbgDumpUnwindTable()
{
	UINT32_PTR_T	pIdx = &__start_exidx;
	UINT32_PTR_T	pIdxHigh;
	UINT8_PTR_T		pTable;
	UINT32_T		uiFuncAddr;
	UINT32_T		uRegMask;
	UINT32_T		uiAddrHigh;

	sUnwind.TypeInfoPtr = 0;

	while (pIdx < &__end_exidx)
	{
		uiFuncAddr = (UINT32_T) pIdx - ((~(*pIdx - 1)) & 0xFFFF);

		if ((uiFuncAddr + 1) == (UINT32_T) &__aeabi_unwind_cpp_pr0) 
		{
			DbgPrintf("__aeabi_unwind_cpp_pr0 :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &__aeabi_unwind_cpp_pr1)
		{
			DbgPrintf("__aeabi_unwind_cpp_pr1 :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &__cxa_allocate_exception)
		{
			DbgPrintf("__cxa_allocate_exception :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &__cxa_begin_catch)
		{
			DbgPrintf("__cxa_begin_catch :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &__cxa_end_catch)
		{
			DbgPrintf("__cxa_end_catch :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &__cxa_end_cleanup)
		{
			DbgPrintf("__cxa_end_cleanup :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &__cxa_free_exception)
		{
			DbgPrintf("__cxa_free_exception :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &__cxa_get_exception_ptr)
		{
			DbgPrintf("__cxa_get_exception_ptr :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &__cxa_throw)
		{
			DbgPrintf("__cxa_throw :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &__cxa_rethrow)
		{
			DbgPrintf("__cxa_rethrow :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &__cxa_call_unexpected)
		{
			DbgPrintf("__cxa_call_unexpected :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &__cxa_call_terminate)
		{
			DbgPrintf("__cxa_call_terminate :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &__gxx_personality_v0)
		{
			DbgPrintf("__gxx_personality_v0 :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &FindUnwindData)
		{
			DbgPrintf("FindUnwindData :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &GetLandingAddress)
		{
			DbgPrintf("GetLandingAddress :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &GetUnwindData)
		{
			DbgPrintf("GetUnwindData :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &RaiseException)
		{
			DbgPrintf("RaiseException :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &main)
		{
			DbgPrintf("main :");
		}
		else if ((uiFuncAddr + 1) == (UINT32_T) &DbgDumpUnwindTable)
		{
			DbgPrintf("DbgDumpUnwindTable :");
		}
		else
		{
			DbgPrintf("Address 0x%4X :", uiFuncAddr);
		}

		pIdxHigh = pIdx;
		pIdxHigh += 2;
		uiAddrHigh = (UINT32_T) pIdxHigh - ((~(*pIdxHigh - 1)) & 0xFFFF);

		pIdx++;

		if (*pIdx <= 0x1) {
			DbgPrintf(" [No unwind data]\r\n");
		} else {
			if ((*pIdx & BITHEX_31) == BITHEX_31) {
				sParams.Result = GetUnwindData(pIdx, &sUnwind, TRUE);
				if (sParams.Result) {
					GetLandingAddress(uiFuncAddr, uiAddrHigh, 0, &sUnwind, TRUE);
				}
			} else if ((*pIdx & 0x70000000) == 0x70000000) {
				pTable = ((UINT8_PTR_T) pIdx) - ((~(*pIdx - 1)) & 0xFFFF);
				sParams.Result = GetUnwindData((UINT32_PTR_T) pTable, &sUnwind, TRUE);
				if (sParams.Result) {
					GetLandingAddress(uiFuncAddr, uiAddrHigh, 0, &sUnwind, TRUE);
				}
			} else {
				DbgPrintf(" [Invalid unwind data = 0x%X]\r\n", *pIdx);
				sParams.Result = FALSE;
			}

			if (sParams.Result) {
				DbgPrintf("\r\n");
				DbgPrintf("   Frame register           : R%d\r\n", sUnwind.FrameReg);
				if (sUnwind.StackDelta) {
					DbgPrintf("   Stack delta              : %d\r\n", sUnwind.StackDelta);
				}
				DbgPrintf("   Pop register(s)          : ");
				if (sUnwind.PopRegs) {
					for (int i = 0; i < 12; i++) {
						uRegMask = 0x1;
						uRegMask = uRegMask << i;
						if (sUnwind.PopRegs & uRegMask) {
							DbgPrintf("{R%d}", (i+4));
						}
					}
				} else {
					DbgPrintf("[None]");
				}
				DbgPrintf("\r\n");
				if (sUnwind.HasGxxPersonality) {
					DbgPrintf("   First landing address    : 0x%X\r\n", sUnwind.LandingAddr);
					DbgPrintf("   Number of handlers       : %d\r\n", sUnwind.NumbOfHandlers);
					DbgPrintf("   Number of landing addr.  : %d\r\n", sUnwind.NumbOfLandingAddr);
				}
			} else {
				DbgPrintf(" [Can't get unwind data]\r\n");
			}
		}

		pIdx++;
	}
}

void DbgDumpUnwindData()
{
	UINT16_T uRegMask;

	DbgPrintf("   Link register            : 0x%X\r\n", sUnwind.LinkRegister);
	DbgPrintf("   Type info pointer        : 0x%X\r\n", sUnwind.TypeInfoPtr);
	DbgPrintf("   Exception pointer        : 0x%X\r\n", sUnwind.ExceptionPtr);
	DbgPrintf("   Frame register           : R%d\r\n", sUnwind.FrameReg);
	DbgPrintf("   Stack delta              : %d\r\n", sUnwind.StackDelta);
	DbgPrintf("   Pop register(s)          : ");
	if (sUnwind.PopRegs) {
		for (int i = 0; i < 12; i++) {
			uRegMask = 0x1;
			uRegMask = uRegMask << i;
			if (sUnwind.PopRegs & uRegMask) {
				DbgPrintf("{R%d}", (i+4));
			}
		}
	} else {
		DbgPrintf("[None]");
	}
	DbgPrintf("\r\n");
	if (sUnwind.HasGxxPersonality) {
		DbgPrintf("   Has GXX personality      : Yes\r\n");
		DbgPrintf("   First landing address    : 0x%X\r\n", sUnwind.LandingAddr);
		DbgPrintf("   Number of handlers       : %d\r\n", sUnwind.NumbOfHandlers);
		DbgPrintf("   Number of landing addr.  : %d\r\n", sUnwind.NumbOfLandingAddr);
	} else {
		DbgPrintf("   Has GXX personality      : No\r\n");
	}
}

};	// End of extern "C"
