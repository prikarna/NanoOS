/*
 * File    : Usb.h
 * Remark  : Macro to access Universal Serial Bus (USB) controller on chip.
 *           Fow now this is STM32F10XXX chipset.
 *
 */

#ifndef USB_H
#define USB_H

#include "..\Type.h"
#include "..\Bit.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USB_BASE		0x40005C00		// USB Full Speed device registers

#define USB_EP0R		0x40005C00		// (USB_BASE + 0x00)
#define USB_EP1R		0x40005C04		// (USB_BASE + 0x04)
#define USB_EP2R		0x40005C08		// (USB_BASE + 0x08)
#define USB_EP3R		0x40005C0C		// (USB_BASE + 0x0C)
#define USB_EP4R		0x40005C10		// (USB_BASE + 0x10)
#define USB_EP5R		0x40005C14		// (USB_BASE + 0x14)
#define USB_EP6R		0x40005C18		// (USB_BASE + 0x18)
#define USB_EP7R		0x40005C1C		// (USB_BASE + 0x1C)

#define USB_CNTR		0x40005C40		// (USB_BASE + 0x40)
#define USB_ISTR		0x40005C44		// (USB_BASE + 0x44)
#define USB_FNR			0x40005C48		// (USB_BASE + 0x48)
#define USB_DADDR		0x40005C4C		// (USB_BASE + 0x4C)
#define USB_BTABLE		0x40005C50		// (USB_BASE + 0x50)

#define USB_BUFF_ADDR	0x40006000

#define USB_EP_REG_0		0
#define USB_EP_REG_1		1
#define USB_EP_REG_2		2
#define USB_EP_REG_3		3

#define USB_CTL__EN_CORECT_TRANS_INT			BITHEX_15
#define USB_CTL__EN_PACK_MEM_OVR_UND_INT		BITHEX_14
#define USB_CTL__EN_ERROR_INT					BITHEX_13
#define USB_CTL__EN_WAKE_UP_INT					BITHEX_12
#define USB_CTL__EN_SUSPEND_INT					BITHEX_11
#define USB_CTL__EN_RESET_INT					BITHEX_10
#define USB_CTL__EN_START_OF_FRAME_INT			BITHEX_9
#define USB_CTL__EN_EXPECT_START_OF_FRAME_INT	BITHEX_8
#define USB_CTL__RESUME							BITHEX_4
#define USB_CTL__FORCE_SUSPEND					BITHEX_3
#define USB_CTL__LOW_POWER_MODE					BITHEX_2
#define USB_CTL__POWER_DOWN						BITHEX_1
#define USB_CTL__FORCE_USB_RESET				BITHEX_0

#define USB_SET_CTL(Set, UsbCtls)					\
	(Set) ? (IO_MEM32(USB_CNTR) |= (UsbCtls)) : (IO_MEM32(USB_CNTR) &= ~(UsbCtls))

#define USB_GET_CTL()							IO_MEM32(USB_CNTR)

#define USB_ENA_REQ_RESUME(Ena)						ENABLE_IO_BIT(USB_CNTR, BIT_4, Ena);

#define USB_SET_FORCE_SUSPEND(Set)					ENABLE_IO_BIT(USB_CNTR, BIT_3, Set)

#define USB_GET_CORRECT_TRANS_INT_STATUS()			(IO_MEM32(USB_ISTR) & BITHEX_15) ? 1 : 0

#define USB_GET_PACK_MEM_OVR_UND_INT_STATUS()		(IO_MEM32(USB_ISTR) & BITHEX_14) ? 1 : 0
#define USB_CLR_PACK_MEM_OVR_UND_RUN_INT()			IO_MEM32(USB_ISTR) = 0xFFFFBFFF

#define USB_GET_ERR_INT_STATUS()					(IO_MEM32(USB_ISTR) & BITHEX_13) ? 1 : 0
#define USB_CLR_ERR_INT()							IO_MEM32(USB_ISTR) = 0xFFFFDFFF

#define USB_GET_WAKE_UP_INT_STATUS()				(IO_MEM32(USB_ISTR) & BITHEX_12) ? 1 : 0
#define USB_CLR_WAKE_UP_INT()						IO_MEM32(USB_ISTR) = 0xFFFFEFFF

#define USB_GET_SUSPEND_INT_STATUS()				(IO_MEM32(USB_ISTR) & BITHEX_11) ? 1 : 0
#define USB_CLR_SUSPEND_INT()						IO_MEM32(USB_ISTR) = 0xFFFFF7FF

#define USB_GET_RESET_INT_STATUS()					(IO_MEM32(USB_ISTR) & BITHEX_10) ? 1 : 0
#define USB_CLR_RESET_INT()							IO_MEM32(USB_ISTR) = 0xFFFFFBFF

#define USB_GET_START_OF_FRAME_INT_STATUS()			(IO_MEM32(USB_ISTR) & BITHEX_9) ? 1 : 0
#define USB_CLR_START_OF_FRAME_INT()				IO_MEM32(USB_ISTR) = 0xFFFFFDFF

#define USB_GET_EXPECT_START_OF_FRAME_INT_STATUS()	(IO_MEM32(USB_ISTR) & BITHEX_8) ? 1 : 0
#define USB_CLR_EXPECT_START_OF_FRAME_INT()			IO_MEM32(USB_ISTR) = 0xFFFFFEFF

#define USB_CLR_ALL_INTS()							IO_MEM32(USB_ISTR) = 0x0000

#define USB_INT__CORRECT_TRANS					BITHEX_15
#define USB_INT__PACK_MEM_OVR_UND_RUN			BITHEX_14
#define USB_INT__ERROR							BITHEX_13
#define USB_INT__WAKEUP							BITHEX_12
#define USB_INT__SUSPEND						BITHEX_11
#define USB_INT__RESET							BITHEX_10
#define USB_INT__START_OF_FRAME					BITHEX_9
#define USB_INT__EXPECT_START_OF_FRAME			BITHEX_8

#define USB_GET_ALL_INTS()						(IO_MEM32(USB_ISTR) & 0xFF00)

#define USB_GET_TRANSACT_DIR()					GET_IO_BIT(USB_ISTR, BIT_4)

#define USB_GET_TRANSACT_EP_ID()				GET_IO_BITS(USB_ISTR, BIT_0, BITMASK_4)

#define USB_GET_RXDP_STATUS()					GET_IO_BIT(USB_FNR, BIT_15)

#define USB_GET_RXDM_STATUS()					GET_IO_BIT(USB_FNR, BIT_14)

#define USB_GET_LOCK_STATUS()					GET_IO_BIT(USB_FNR, BIT_13)

#define USB_GET_LOST_START_OF_FRAME()			GET_IO_BITS(USB_FNR, BIT_11, BITMASK_2)

#define USB_GET_FRAME_NUMB()					(IO_MEM32(USB_FNR) & 0x7FF)

#define USB_ENABLE(Ena)							\
	(Ena) ? (IO_MEM32(USB_DADDR) |= BITHEX_7) : (IO_MEM32(USB_DADDR) &= ~(BITHEX_7))

#define USB_SET_DEV_ADDR(UsbAddr)				\
	IO_MEM32(USB_DADDR) = (IO_MEM32(USB_DADDR) & ~(0x3F)) | (UsbAddr & 0x7F)

#define USB_SET_BTBL_ADDR(UsbBTblAddr)			IO_MEM32(USB_BTABLE) = UsbBTblAddr & 0xFFFF
#define USB_GET_BTBL_ADDR()						(IO_MEM32(USB_BTABLE))

#define USB_EP_TYPE__BULK						0x0
#define USB_EP_TYPE__CONTROL					0x1
#define USB_EP_TYPE__ISO						0x2
#define USB_EP_TYPE__INTERRUPT					0x3

#define USB_EP_KIND__NONE						0x0
#define USB_EP_KIND__DOUBLE_BUF					0x1
#define USB_EP_KIND__STATUS_OUT					0x1

#define USB_EP_STAT__DISABLED					0x0
#define USB_EP_STAT__STALL						0x1
#define USB_EP_STAT__NAK						0x2
#define USB_EP_STAT__VALID						0x3

#define USB_EP_REG_ADDR(EpRegNo)				(USB_EP0R + (EpRegNo * 4))

#define USB_EP_GET_REG(EpRegNo)					(IO_MEM32(USB_EP_REG_ADDR(EpRegNo)))

#define USB_EP_SET(EpRegNo, EpType, EpKind, EpAddr)					\
	IO_MEM32(USB_EP_REG_ADDR(EpRegNo)) = (BITHEX_15 | ((EpType & 0x3) << 9) | ((EpKind & 0x1) << 8) | BITHEX_7 | (EpAddr & 0xF))

#define USB_EP_GET_RX_CTR(EpRegNo)				GET_IO_BIT(USB_EP_REG_ADDR(EpRegNo), BIT_15)
#define USB_EP_GET_RX_SYNC_DAT(EpRegNo)			GET_IO_BIT(USB_EP_REG_ADDR(EpRegNo), BIT_14)
#define USB_EP_TOG_RX_SYNC_DAT(EpRegNo)			SET_IO_BIT(USB_EP_REG_ADDR(EpRegNo), BIT_14)
#define USB_EP_GET_RX_STAT(EpRegNo)				GET_IO_BITS(USB_EP_REG_ADDR(EpRegNo), BIT_12, BITMASK_2)

#define USB_EP_IS_SETUP(EpRegNo)				GET_IO_BIT(USB_EP_REG_ADDR(EpRegNo), BIT_11)

#define USB_EP_GET_TYPE(EpRegNo)				GET_IO_BITS(USB_EP_REG_ADDR(EpRegNo), BIT_9, BITMASK_2)

#define USB_EP_GET_KIND(EpRegNo)				GET_IO_BIT(USB_EP_REG_ADDR(EpRegNo), BIT_8)

#define USB_EP_GET_TX_CTR(EpRegNo)				GET_IO_BIT(USB_EP_REG_ADDR(EpRegNo), BIT_7)
#define USB_EP_GET_TX_SYNC_DAT(EpRegNo)			GET_IO_BIT(USB_EP_REG_ADDR(EpRegNo), BIT_6)
#define USB_EP_TOG_TX_SYNC_DAT(EpRegNo)			SET_IO_BIT(USB_EP_REG_ADDR(EpRegNo), BIT_6)
#define USB_EP_GET_TX_STAT(EpRegNo)				GET_IO_BITS(USB_EP_REG_ADDR(EpRegNo), BIT_4, BITMASK_2)

#define USB_EP_GET_ADDR(EpRegNo)				GET_IO_BITS(USB_EP_REG_ADDR(EpRegNo), BIT_0, BITMASK_4)

#define USB_EP_SET_TX_BUF_ADDR(EpRegNo, BTbl, Addr)			\
	IO_MEM32((USB_BUFF_ADDR + BTbl + (EpRegNo * 16))) = (Addr & BITMASK_16)

#define USB_EP_GET_TX_BUF_ADDR(EpRegNo, BTbl)				\
	(USB_BUFF_ADDR + ((IO_MEM32((USB_BUFF_ADDR + BTbl + (EpRegNo * 16)))) * 2))

#define USB_EP_SET_TX_BUF_LEN(EpRegNo, BTbl, Len)			\
	IO_MEM32((USB_BUFF_ADDR + BTbl + (EpRegNo * 16) + 4)) = (Len & BITMASK_10)

#define USB_EP_SET_RX_BUF_ADDR(EpRegNo, BTbl, Addr)			\
	IO_MEM32((USB_BUFF_ADDR + BTbl + (EpRegNo * 16) + 8)) = (Addr & BITMASK_16)

#define USB_EP_GET_RX_BUF_ADDR(EpRegNo, BTbl)				\
	(USB_BUFF_ADDR + ((IO_MEM32((USB_BUFF_ADDR + BTbl + (EpRegNo * 16) + 8))) * 2))

#define USB_EP_SET_RX_BUF_LEN(EpRegNo, BTbl, BlockSz, NumbOfBlock)				\
	IO_MEM32((USB_BUFF_ADDR + BTbl + (EpRegNo * 16) + 12)) =					\
	(IO_MEM32((USB_BUFF_ADDR + BTbl + (EpRegNo * 16) + 12)) & ~(0x3F << 10)) |	\
	((((BlockSz & 0x1) << 5) | (NumbOfBlock & 0x1F)) << 10)

#define USB_EP_GET_RX_DAT_COUNT(EpRegNo, BTbl)									\
	((IO_MEM32((USB_BUFF_ADDR + BTbl + (EpRegNo * 16) + 12))) & 0x3FF)

/*
	USB_EP_CTL__CONTROL_IN_AND_OUT_INIT => To initialize control IN and OUT
		- Keep CTR_RX[15]
		- Keep DTOG_RX[14]
		- Toggle STAT_RX[13:12], DISABLE -> STALL
		- Set EP_TYPE[10:9] to control transfer (1)
		- Set EP_KIND[8] to none (0)
		- Keep CTR_TX[7]
		- Keep DTOG_TX[6]
		- Toggle STAT_TX[5:4], DISABLE -> STALL
		- Set EA to endpoint 0
*/
#define USB_EP_CTL__CONTROL_IN_AND_OUT_INIT		(BITHEX_15 | BITHEX_12 | (USB_EP_TYPE__CONTROL << 9) | BITHEX_7 | BITHEX_4)

/*
	USB_EP_CTL__CONTROL_IN => To set setup IN on control transfer
		- Clear CTR_RX[15]
		- Keep DTOG_RX[14]
		- Keep STAT_RX[13:12], NACK
		- Set EP_TYPE[10:9] to control transfer (1)
		- Set EP_KIND[8] to none (0)
		- Keep CTR_TX[7]
		- Keep DTOG_TX[6]
		- Toggle STAT_TX[5:4], NACK -> VALID
		- Set EA to endpoint 0
*/
#define USB_EP_CTL__CONTROL_IN					((USB_EP_TYPE__CONTROL << 9) | BITHEX_7 | BITHEX_4)

/*
	USB_EP_CTL__CONTROL_OUT => To set setup OUT on control transfer
		- Clear CTR_RX[15]
		- Keep DTOG_RX[14]
		- Toggle STAT_RX[13:12], NACK -> VALID
		- Set EP_TYPE[10:9] to control transfer (1)
		- Set EP_KIND[8] to none (0)
		- Keep CTR_TX[7]
		- Keep DTOG_TX[6]
		- Keep STAT_TX[5:4], NACK
		- Set EA to endpoint 0
*/
#define USB_EP_CTL__CONTROL_OUT					(BITHEX_12 | (USB_EP_TYPE__CONTROL << 9) | BITHEX_7)

/*
	USB_EP_CTL__CONTROL_IN_STALL => To set setup IN to stall on control transfer
		- Clear CTR_RX[15]
		- Keep DTOG_RX[14]
		- Keep STAT_RX[13:12], NACK
		- Set EP_TYPE[10:9] tp control transfer (1)
		- Set EP_KIND[8] to none (0)
		- Clear CTR_TX[7]
		- Keep DTOG_TX[6]
		- Toggle STAT_TX[5:4], NACK -> STALL
		- Set EA to endpoint 0
*/
#define USB_EP_CTL__CONTROL_IN_STALL			((USB_EP_TYPE__CONTROL << 9) | (0x3 << 4))

/*
	USB_EP_CTL__CONTROL_OUT_STALL => To set setup OUT to stall on control transfer
		- Clear CTR_RX[15]
		- Keep DTOG_RX[14]
		- Toggle STAT_RX[13:12], NACK -> STALL
		- Set EP_TYPE[10:9] tp control transfer (1)
		- Set EP_KIND[8] to none (0)
		- Clear CTR_TX[7]
		- Keep DTOG_TX[6]
		- Keep STAT_TX[5:4], NACK
		- Set EA to endpoint 0
*/
#define USB_EP_CTL__CONTROL_OUT_STALL			((0x3 << 4) | (USB_EP_TYPE__CONTROL << 9))

/*
	USB_EP_CTL__CONTROL_IN_TO_OUT => To set setup IN to OUT on control transfer
		- Keep CTR_RX[15]
		- Keep DTOG_RX[14]
		- Toggle STAT_RX[13:12], NACK -> VALID
		- Set EP_TYPE[10:9] to control transfer (1)
		- Set EP_KIND[8] to status out (1)
		- Clear CTR_TX[7]
		- Keep DTOG_TX[6]
		- Keep STAT_TX[5:4]
		- Set EA to endpoint 0
*/
#define USB_EP_CTL__CONTROL_IN_TO_OUT			(BITHEX_15 | BITHEX_12 | (USB_EP_TYPE__CONTROL << 9) | (USB_EP_KIND__STATUS_OUT << 8))

/*
	USB_EP_CTL__CONTROL_OUT_TO_IN => To set setup OUT to IN on control transfer
		- Clear CTR_RX[15]
		- Keep DTOG_RX[14]
		- Keep STAT_RX[13:12], NACK
		- Set EP_TYPE[10:9] to control transfer (1)
		- Set EP_KIND[8] to none (0)
		- Keep CTR_TX[7]
		- Keep DTOG_TX[6]
		- Keep STAT_TX[5:4], NACK -> VALID
		- Set EA to endpoint 0
*/
#define USB_EP_CTL__CONTROL_OUT_TO_IN			((USB_EP_TYPE__CONTROL << 9) | BITHEX_7 | BITHEX_4)

/*
	USB_EP_CTL__CONTROL_OUT_END => To set setup OUT-END (end transfer) on control transfer
		- Clear CTR_RX[15]
		- Keep DTOG_RX[14]
		- Keep STAT_RX[13:12]
		- Set EP_TYPE[10:9] to control transfer (1)
		- Set EP_KIND[8] to none (0)
		- Keep CTR_TX[7]
		- Keep DTOG_TX[6]
		- Keep STAT_TX[5:4]
		- Set EA to endpoint 0
*/
#define USB_EP_CTL__CONTROL_OUT_END					((USB_EP_TYPE__CONTROL << 9) | BITHEX_7)

/*
	USB_EP_CTL__CONTROL_IN_END => To set setup IN-END (end transfer) on control transfer
		- Keep CTR_RX[15]
		- Keep DTOG_RX[14]
		- Keep STAT_RX[13:12], NACK
		- Set EP_TYPE[10:9] to control transfer (1)
		- Set EP_KIND[8] to none (0)
		- Clear CTR_TX[7]
		- Keep DTOG_TX[6]
		- Toggle STAT_TX[5:4], NACK -> VALID
		- Set EA to endpoint 0
*/
#define USB_EP_CTL__CONTROL_IN_END					(BITHEX_15 | (USB_EP_TYPE__CONTROL << 9) | BITHEX_4)

#define USB_EP_CTL(EpRegNo, EpCtlCode)				IO_MEM32(USB_EP_REG_ADDR(EpRegNo)) = EpCtlCode

/*
	USB_EP_CTL_EX__BULK_IN_INIT => To initialize Bulk IN transfer
		- Keep CTR_RX[15]
		- Keep DTOG_RX[14]
		- Keep STAT_RX[13:12]
		- Set EP_TYPE[10:9] to bulk transfer (0)
		- Set EP_KIND[8] to none (0)
		- Keep CTR_TX[7]
		- Keep DTOG_TX[6]
		- Toggle STAT_TX[5:4], DISABLE -> NAK
*/
#define USB_EP_CTL_EX__BULK_IN_INIT					(BITHEX_15 | BITHEX_7 | (0x02 << 4))
//#define USB_EP_CTL_EX__BULK_IN_INIT					(BITHEX_15 | BITHEX_7 | (0x03 << 4))

/*
	USB_EP_CTL_EX__BULK_OUT_INIT => To initialize Bulk OUT transfer
		- Keep CTR_RX[15]
		- Keep DTOG_RX[14],
		- Toggle STAT_RX[13:12], DISABLE -> VALID
		- Set EP_TYPE[10:9] to bulk transfer (0)
		- Set EP_KIND[8] to none (0)
		- Keep CTR_TX[7]
		- Keep DTOG_TX[6]
		- Keep STAT_TX[5:4]
*/
#define USB_EP_CTL_EX__BULK_OUT_INIT				(BITHEX_15 | (0x3 << 12) | BITHEX_7)

/*
	USB_EP_CTL_EX__BULK_IN_CLEAR => To set Bulk IN to clear TX correct transfer interrupt
		- Keep CTR_RX[15]
		- Keep DTOG_RX[14]
		- Keep STAT_RX[13:12]
		- Set EP_TYPE[10:9] to bulk transfer (0)
		- Set EP_KIND[8] to none (0)
		- Clear CTR_TX[7]
		- Keep DTOG_TX[6]
		- Keep STAT_TX[5:4]
*/
#define USB_EP_CTL_EX__BULK_IN_CLEAR			(BITHEX_15)
//#define USB_EP_CTL_EX__BULK_IN_CLEAR_VALID		(BITHEX_15 | BITHEX_7 | BITHEX_4)
#define USB_EP_CTL_EX__BULK_IN_CLEAR_STALL		(BITHEX_15 | BITHEX_4)

/*
	USB_EP_CTL_EX__BULK_IN_VALID => To set Bulk IN transfer after process the data
		- Keep CTR_RX[15]
		- Keep DTOG_RX[14]
		- Keep STAT_RX[13:12]
		- Set EP_TYPE[10:9] to bulk transfer (0)
		- Set EP_KIND[8] to none (0)
		- Keep CTR_TX[7]
		- Keep DTOG_TX[6]
		- Toggle STAT_TX[5:4], NACK -> VALID
*/
#define USB_EP_CTL_EX__BULK_IN_VALID				(BITHEX_15 | BITHEX_7 | BITHEX_4)
#define USB_EP_CTL_EX__BULK_IN_CLEAR_VALID			(BITHEX_15 | BITHEX_4)

/*
	USB_EP_CTL_EX__BULK_OUT_CLEAR => To set Bulk OUT to clear RX correct transfer interrupt
		- Clear CTR_RX[15]
		- Keep DTOG_RX[14]
		- Keep STAT_RX[13:12]
		- Set EP_TYPE[10:9] to bulk transfer (0)
		- Set EP_KIND[8] to none (0)
		- Keep CTR_TX[7]
		- Keep DTOG_TX[6]
		- Keep STAT_TX[5:4]
*/
#define USB_EP_CTL_EX__BULK_OUT_CLEAR				(BITHEX_7)

/*
	USB_EP_CTL_EX__BULK_OUT_VALID => To set Bulk OUT transfer after copy data to buffer
		- Keep CTR_RX[15]
		- Keep DTOG_RX[14]
		- Toggle STAT_RX[13:12], NACK -> VALID
		- Set EP_TYPE[10:9] to bulk transfer (0)
		- Set EP_KIND[8] to none (0)
		- Keep CTR_TX[7]
		- Keep DTOG_TX[6]
		- Keep STAT_TX[5:4]
*/
#define USB_EP_CTL_EX__BULK_OUT_VALID				(BITHEX_15 | BITHEX_12 | BITHEX_7)

/*
	USB_EP_CTL_EX__BULK_OUT_CLEAR_VALID => To set Bulk OUT transfer after copy data to buffer
		- Clear CTR_RX[15]
		- Keep DTOG_RX[14]
		- Toggle STAT_RX[13:12], NACK -> VALID
		- Set EP_TYPE[10:9] to bulk transfer (0)
		- Set EP_KIND[8] to none (0)
		- Keep CTR_TX[7]
		- Keep DTOG_TX[6]
		- Keep STAT_TX[5:4]
*/
#define USB_EP_CTL_EX__BULK_OUT_CLEAR_VALID			(BITHEX_12 | BITHEX_7)
#define USB_EP_CTL_EX__BULK_OUT_CLEAR_VALID_TOGGLE	(BITHEX_14 | BITHEX_12 | BITHEX_14)

/*
	USB_EP_CTL_EX__INTERRUPT_IN_INIT => To initialize Interrupt IN transfer
		- Keep CTR_RX[15]
		- Keep DTOG_RX[14]
		- Keep STAT_RX[13:12]
		- Set EP_TYPE[10:9] to interrupt transfer (3)
		- Set EP_KIND[8] to none (0)
		- Keep CTR_TX[7]
		- Toggle DTOG_TX[6], 0 -> 1
		- Toggle STAT_TX[5:4], DISABLE -> VALID
*/
#define USB_EP_CTL_EX__INTERRUPT_IN_INIT		(BITHEX_15 | (USB_EP_TYPE__INTERRUPT << 9) | BITHEX_7 | BITHEX_6 | (0x03 << 4))

/*
	USB_EP_CTL_EX__INTERRUPT_IN_CLEAR => To set Interrupt IN transfer before process the data
		- Keep CTR_RX[15]
		- Keep DTOG_RX[14]
		- Keep STAT_RX[13:12]
		- Set EP_TYPE[10:9] to interrupt transfer (3)
		- Set EP_KIND[8] to none (0)
		- Clear CTR_TX[7]
		- Keep DTOG_TX[6]
		- Keep STAT_TX[5:4]
*/
#define USB_EP_CTL_EX__INTERRUPT_IN_CLEAR		(BITHEX_15 | (USB_EP_TYPE__INTERRUPT << 9))

/*
	USB_EP_CTL_EX__INTERRUPT_IN_VALID => To set Interrupt IN transfer after process the data
		- Keep CTR_RX[15]
		- Keep DTOG_RX[14]
		- Keep STAT_RX[13:12]
		- Set EP_TYPE[10:9] to interrupt transfer (3)
		- Set EP_KIND[8] to none (0)
		- Keep CTR_TX[7]
		- Keep DTOG_TX[6]
		- Toggle STAT_TX[5:4], NACK -> VALID
*/
#define USB_EP_CTL_EX__INTERRUPT_IN_VALID		(BITHEX_15 | (USB_EP_TYPE__INTERRUPT << 9) | BITHEX_7 | BITHEX_4)

#define USB_EP_CTL_EX(EpRegNo, EpAddr, EpCtlExCode)			\
	IO_MEM32(USB_EP_REG_ADDR(EpRegNo)) = (EpCtlExCode | (EpAddr & 0xF))

#ifdef __cplusplus
}
#endif

#endif  // End of USB_H
