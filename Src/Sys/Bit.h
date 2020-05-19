/*
 * File    : Bit.h
 * Remark  : Bit or bits definition and macro to manipulate bit(s) 
 *           on an address.
 *
 */

#ifndef BIT_H
#define BIT_H

#ifdef __cplusplus
extern "C" {
#endif

#define BITHEX_0			0x00000001
#define BITHEX_1			0x00000002
#define BITHEX_2			0x00000004
#define BITHEX_3			0x00000008
#define BITHEX_4			0x00000010
#define BITHEX_5			0x00000020
#define BITHEX_6			0x00000040
#define BITHEX_7			0x00000080
#define BITHEX_8			0x00000100
#define BITHEX_9			0x00000200
#define BITHEX_10			0x00000400
#define BITHEX_11			0x00000800
#define BITHEX_12			0x00001000
#define BITHEX_13			0x00002000
#define BITHEX_14			0x00004000
#define BITHEX_15			0x00008000
#define BITHEX_16			0x00010000
#define BITHEX_17			0x00020000
#define BITHEX_18			0x00040000
#define BITHEX_19			0x00080000
#define BITHEX_20			0x00100000
#define BITHEX_21			0x00200000
#define BITHEX_22			0x00400000
#define BITHEX_23			0x00800000
#define BITHEX_24			0x01000000
#define BITHEX_25			0x02000000
#define BITHEX_26			0x04000000
#define BITHEX_27			0x08000000
#define BITHEX_28			0x10000000
#define BITHEX_29			0x20000000
#define BITHEX_30			0x40000000
#define BITHEX_31			0x80000000

#define BITMASK_1			0x00000001
#define BITMASK_2			0x00000003
#define BITMASK_3			0x00000007
#define BITMASK_4			0x0000000F
#define BITMASK_5			0x0000001F
#define BITMASK_6			0x0000003F
#define BITMASK_7			0x0000007F
#define BITMASK_8			0x000000FF
#define BITMASK_9			0x000001FF
#define BITMASK_10			0x000003FF
#define BITMASK_11			0x000007FF
#define BITMASK_12			0x00000FFF
#define BITMASK_13			0x00001FFF
#define BITMASK_14			0x00003FFF
#define BITMASK_15			0x00007FFF
#define BITMASK_16			0x0000FFFF
#define BITMASK_17			0x0001FFFF
#define BITMASK_18			0x0003FFFF
#define BITMASK_19			0x0007FFFF
#define BITMASK_20			0x000FFFFF
#define BITMASK_21			0x001FFFFF
#define BITMASK_22			0x003FFFFF
#define BITMASK_23			0x007FFFFF
#define BITMASK_24			0x00FFFFFF
#define BITMASK_25			0x01FFFFFF
#define BITMASK_26			0x03FFFFFF
#define BITMASK_27			0x07FFFFFF
#define BITMASK_28			0x0FFFFFFF
#define BITMASK_29			0x1FFFFFFF
#define BITMASK_30			0x3FFFFFFF
#define BITMASK_31			0x7FFFFFFF
#define BITMASK_32			0xFFFFFFFF

#define BITMASK_REV_1		0x80000000
#define BITMASK_REV_2		0xC0000000
#define BITMASK_REV_3		0xE0000000
#define BITMASK_REV_4		0xF0000000
#define BITMASK_REV_5		0xF8000000
#define BITMASK_REV_6		0xFC000000
#define BITMASK_REV_7		0xFE000000
#define BITMASK_REV_8		0xFF000000
#define BITMASK_REV_9		0xFF800000
#define BITMASK_REV_10		0xFFC00000
#define BITMASK_REV_11		0xFFE00000
#define BITMASK_REV_12		0xFFF00000
#define BITMASK_REV_13		0xFFF80000
#define BITMASK_REV_14		0xFFFC0000
#define BITMASK_REV_15		0xFFFE0000
#define BITMASK_REV_16		0xFFFF0000
#define BITMASK_REV_17		0xFFFF8000
#define BITMASK_REV_18		0xFFFFC000
#define BITMASK_REV_19		0xFFFFE000
#define BITMASK_REV_20		0xFFFFF000
#define BITMASK_REV_21		0xFFFFF800
#define BITMASK_REV_22		0xFFFFFC00
#define BITMASK_REV_23		0xFFFFFE00
#define BITMASK_REV_24		0xFFFFFF00
#define BITMASK_REV_25		0xFFFFFF80
#define BITMASK_REV_26		0xFFFFFFC0
#define BITMASK_REV_27		0xFFFFFFE0
#define BITMASK_REV_28		0xFFFFFFF0
#define BITMASK_REV_29		0xFFFFFFF8
#define BITMASK_REV_30		0xFFFFFFFC
#define BITMASK_REV_31		0xFFFFFFFE
#define BITMASK_REV_32		0xFFFFFFFF

#define BIT_0				0
#define BIT_1				1
#define BIT_2				2
#define BIT_3				3
#define BIT_4				4
#define BIT_5				5
#define BIT_6				6
#define BIT_7				7
#define BIT_8				8
#define BIT_9				9
#define BIT_10				10
#define BIT_11				11
#define BIT_12				12
#define BIT_13				13
#define BIT_14				14
#define BIT_15				15
#define BIT_16				16
#define BIT_17				17
#define BIT_18				18
#define BIT_19				19
#define BIT_20				20
#define BIT_21				21
#define BIT_22				22
#define BIT_23				23
#define BIT_24				24
#define BIT_25				25
#define BIT_26				26
#define BIT_27				27
#define BIT_28				28
#define BIT_29				29
#define BIT_30				30
#define BIT_31				31

#define IO_MEM32(Address)				*((volatile UINT32_PTR_T) (Address))
#define WRITE_IO_MEM32(Address, Value)	IO_MEM32(Address) = (UINT32_T) Value
#define READ_IO_MEM32(Address)			IO_MEM32(Address)

#define SET_IO_BIT(Address, NthBit)		IO_MEM32(Address) |= (((UINT32_T) 0x1) << NthBit)
#define CLR_IO_BIT(Address, NthBit)		IO_MEM32(Address) &= ~(((UINT32_T) 0x1) << NthBit)
#define GET_IO_BIT(Address, NthBit)		((IO_MEM32(Address) >> NthBit) & ((UINT32_T) 0x1))

#define GET_IO_BITS(Address, NthBit, BitMask)		((IO_MEM32(Address) >> NthBit) & BitMask)

#define SET_IO_BITS(Address, NthBit, BitMask, Val)	\
	IO_MEM32(Address) = ((IO_MEM32(Address) & ~(BitMask << NthBit)) | (( ((UINT32_T) Val) & BitMask) << NthBit))

#define ENABLE_IO_BIT(Address, NthBit, Enable)		\
	(Enable) ? (SET_IO_BIT(Address, NthBit)) : (CLR_IO_BIT(Address, NthBit))

#ifdef __cplusplus
}
#endif

#endif	// End of BIT_H
