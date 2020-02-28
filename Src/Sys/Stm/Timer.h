/*
 * File    : Timer.h
 * Remark  : Macro to access timer controller on chip.
 *           Fow now this is STM32F10XXX chipset.
 *
 */

#ifndef TIMER_H
#define TIMER_H

#include "..\Bit.h"

#define TIM1_BASE				0x40012C00
#define TIM2_BASE				0x40000000
#define TIM3_BASE				0x40000400
#define TIM4_BASE				0x40000800
#define TIM5_BASE				0x40000C00

#define TIM1					TIM1_BASE
#define TIM2					TIM2_BASE
#define TIM3					TIM3_BASE
#define TIM4					TIM4_BASE
#define TIM5					TIM5_BASE

#define TIM_CR1_OFFSET			0x00
#define TIM_CR2_OFFSET			0x04
#define TIM_SMCR_OFFSET			0x08
#define TIM_DIER_OFFSET			0x0C
#define TIM_SR_OFFSET			0x10
#define TIM_EGR_OFFSET			0x14
#define TIM_CCMR1_OFFSET		0x18
#define TIM_CCMR2_OFFSET		0x1C
#define TIM_CCER_OFFSET			0x20
#define TIM_CNT_OFFSET			0x24
#define TIM_PSC_OFFSET			0x28
#define TIM_ARR_OFFSET			0x2C
#define TIM_CCR1_OFFSET			0x34
#define TIM_CCR2_OFFSET			0x38
#define TIM_CCR3_OFFSET			0x3C
#define TIM_CCR4_OFFSET			0x40
#define TIM_DCR_OFFSET			0x48
#define TIM_DMAR_OFFSET			0x4C

#define TIM_CLOCK_DIV__ONE		0x0
#define TIM_CLOCK_DIV__TWO		0x1
#define TIM_CLOCK_DIV__THREE	0x2

#define TIM_SET_CLCK_DIV_FOR_DGT_FLT(TimNo, TimClockDiv)	SET_IO_BITS((TimNo + TIM_CR1_OFFSET), BIT_8, BITMASK_2, TimClockDiv)
#define TIM_ENA_AUTO_RELOAD_BUFFER(TimNo, Enable)			ENABLE_IO_BIT((TimNo + TIM_CR1_OFFSET), BIT_7, Enable)

#define TIM_ALIGN_MOD__EDGE		0x0
#define TIM_ALIGN_MOD__CENTER1	0x1
#define TIM_ALIGN_MOD__CENTER2	0x2
#define TIM_ALIGN_MOD__CENTER3	0x3

#define TIM_SET_ALIGNED_MODE(TimNo, TimAlignMod)			SET_IO_BITS((TimNo + TIM_CR1_OFFSET), BIT_5, BITMASK_2, TimAlignMod)
#define TIM_ENA_DOWNCOUNT(TimNo, Enable)					ENABLE_IO_BIT((TimNo + TIM_CR1_OFFSET), BIT_4, Enable)
#define TIM_ENA_ONE_PULSE_MODE(TimNo, Enable)				ENABLE_IO_BIT((TimNo + TIM_CR1_OFFSET), BIT_3, Enable)
#define TIM_ENA_OVER_OR_UNDERFLOW_UPDT_SRC(TimNo, Enable)	ENABLE_IO_BIT((TimNo + TIM_CR1_OFFSET), BIT_2, Enable)
#define TIM_DISABLE_UPDATE(TimNo, Disabled)					ENABLE_IO_BIT((TimNo + TIM_CR1_OFFSET), BIT_1, Disabled)
#define TIM_ENA_COUNTER(TimNo, Enable)						ENABLE_IO_BIT((TimNo + TIM_CR1_OFFSET), BIT_0, Enable)

#define TIM_ENA_CHAN1_TO_3_CONNECT_TO_TIM_INP1(TimNo, Enable)		\
	ENABLE_IO_BIT((TimNo + TIM_CR2_OFFSET), BIT_7, Enable)

#define TIM_MAST_MOD_SEL__RESET				0x0
#define TIM_MAST_MOD_SEL__ENABLE			0x1
#define TIM_MAST_MOD_SEL__UPDATE			0x2
#define TIM_MAST_MOD_SEL__COMP_PULSE		0x3
#define TIM_MAST_MOD_SEL__COMP_OC1REF		0x4
#define TIM_MAST_MOD_SEL__COMP_OC2REF		0x5
#define TIM_MAST_MOD_SEL__COMP_OC3REF		0x6
#define TIM_MAST_MOD_SEL__COMP_OC4REF		0x7

#define TIM_SET_MASTER_MOD_SEL(TimNo, TimMastModSel)		\
	SET_IO_BITS((TimNo + TIM_CR2_OFFSET), BIT_4, BITMASK_3, TimMastModSel)

#define TIM_ENA_DMA_REQ_WHEN_UPDATE_EVENT(TimNo, Enable)	ENABLE_IO_BIT((TimNo + TIM_CR2_OFFSET), BIT_3, Enable)
#define TIM_ENA_INVERT_EXTERN_TRIG_POL(TimNo, Enable)		ENABLE_IO_BIT((TimNo + TIM_SMCR_OFFSET), BIT_15, Enable)
#define TIM_ENA_EXTERN_CLK(TimNo, Enable)					ENABLE_IO_BIT((TimNo + TIM_SMCR_OFFSET), BIT_14, Enable)

#define TIM_EXTERN_TRIG_PRESCALE__OFF				0x0
#define TIM_EXTERN_TRIG_PRESCALE__DIV_BY_2			0x1
#define TIM_EXTERN_TRIG_PRESCALE__DIV_BY_4			0x2
#define TIM_EXTERN_TRIG_PRESCALE__DIV_BY_8			0x3

#define TIM_SET_EXTERN_TRIG_PRESCLA(TimNo, TimExternTrigScaler)			\
	SET_IO_BITS((TimNo + TIM_SMCR_OFFSET), BIT_12, BITMASK_2, TimExternTrigScaler)

#define TIM_EXTERN_TRIG_FILT__NONE			0x0
#define TIM_EXTERN_TRIG_FILT__1				0x1
#define TIM_EXTERN_TRIG_FILT__2				0x2
#define TIM_EXTERN_TRIG_FILT__3				0x3
#define TIM_EXTERN_TRIG_FILT__4				0x4
#define TIM_EXTERN_TRIG_FILT__5				0x5
#define TIM_EXTERN_TRIG_FILT__6				0x6
#define TIM_EXTERN_TRIG_FILT__7				0x7
#define TIM_EXTERN_TRIG_FILT__8				0x8
#define TIM_EXTERN_TRIG_FILT__9				0x9
#define TIM_EXTERN_TRIG_FILT__10			0xA
#define TIM_EXTERN_TRIG_FILT__11			0xB
#define TIM_EXTERN_TRIG_FILT__12			0xC
#define TIM_EXTERN_TRIG_FILT__13			0xD
#define TIM_EXTERN_TRIG_FILT__14			0xE
#define TIM_EXTERN_TRIG_FILT__15			0xF

#define TIM_SET_EXTERN_TRIG_FILTER(TimNo, TimExternTrigFilt)			\
	SET_IO_BITS((TimNo + TIM_SMCR_OFFSET), BIT_8, BITMASK_4, TimExternTrigFilt)

#define TIM_ENA_MAST_SLAV_MOD(TimNo, Enable)				ENABLE_IO_BIT((TimNo + TIM_SMCR_OFFSET), BIT_7, Enable)

#define TIM_TRIG_SEL__INT_TRIG0						0x0
#define TIM_TRIG_SEL__INT_TRIG1						0x1
#define TIM_TRIG_SEL__INT_TRIG2						0x2
#define TIM_TRIG_SEL__INT_TRIG3						0x3
#define TIM_TRIG_SEL__TIM_INP1_EDGE_DET				0x4
#define TIM_TRIG_SEL__FILT_TIM_INP1					0x5
#define TIM_TRIG_SEL__FILT_TIM_INP2					0x6
#define TIM_TRIG_SEL__EXTER_TRIG_INP				0x7

#define TIM_SET_TRIG_SELECTION(TimNo, TimTrigSel)					\
	SET_IO_BITS((TimNo + TIM_SMCR_OFFSET), BIT_4, BITMASK_3, TimTrigSel)

#define TIM_CHAN1						0x1
#define TIM_CHAN2						0x2
#define TIM_CHAN3						0x3
#define TIM_CHAN4						0x4

#define TIM_ENA_TRIG_DMA_REQ(TimNo, Enable)					ENABLE_IO_BIT((TimNo + TIM_DIER_OFFSET), BIT_14, Enable)

#define TIM_ENA_CAPT_COMP_DMA_REQ(TimNo, TimChanNo, Enable)			\
	ENABLE_IO_BIT((TimNo + TIM_DIER_OFFSET), (BIT_8 + TimChanNo), Enable)

#define TIM_ENA_UPDATE_DMA_REQ(TimNo, Enable)				ENABLE_IO_BIT((TimNo + TIM_DIER_OFFSET), BIT_8, Enable)
#define TIM_ENA_TRIG_INTERRUPT(TimNo, Enable)				ENABLE_IO_BIT((TimNo + TIM_DIER_OFFSET), BIT_6, Enable)
#define TIM_ENA_UPDATE_INTERRUPT(TimNo, Enable)				ENABLE_IO_BIT((TimNo + TIM_DIER_OFFSET), BIT_0, Enable)

#define TIM_ENA_CAPT_COMP_INT(TimNo, TimChanNo, Enable)			\
	ENABLE_IO_BIT((TimNo + TIM_DIER_OFFSET), (BIT_0 + TimChanNo), Enable)

#define TIM_ENA_UPDATE_INT(TimNo, Enable)					ENABLE_IO_BIT((TimNo + TIM_DIER_OFFSET), BIT_0, Enable)

#define TIM_GET_OVR_CAPT_FLAG(TimNo, TimChanNo)				GET_IO_BIT((TimNo + TIM_SR_OFFSET), (BIT_8 + TimChanNo))
#define TIM_CLR_OVR_CAPT_FLAG(TimNo, TimChanNo)				CLR_IO_BIT((TimNo + TIM_SR_OFFSET), (BIT_8 + TimChanNo))
#define TIM_GET_INT_TRIG_FLAG(TimNo)						GET_IO_BIT((TimNo + TIM_SR_OFFSET), BIT_6)
#define TIM_CLR_INT_TRIG_FLAG(TimNo)						CLR_IO_BIT((TimNo + TIM_SR_OFFSET), BIT_6)
#define TIM_GET_CAPT_COMP_INT_FLAG(TimNo, TimChanNo)		GET_IO_BIT((TimNo + TIM_SR_OFFSET), (BIT_0 + TimChanNo))
#define TIM_CLR_CAPT_COMP_INT_FLAG(TimNo, TimChanNo)		CLR_IO_BIT((TimNo + TIM_SR_OFFSET), (BIT_0 + TimChanNo))
#define TIM_GET_UPDATE_INT_FLAG(TimNo)						GET_IO_BIT((TimNo + TIM_SR_OFFSET), BIT_0)
#define TIM_CLR_UPDATE_INT_FLAG(TimNo)						CLR_IO_BIT((TimNo + TIM_SR_OFFSET), BIT_0)

#define TIM_SET_TRIG_GEN(TimNo)								SET_IO_BIT((TimNo + TIM_EGR_OFFSET), BIT_6)
#define TIM_SET_CAPT_COMP_GEN(TimNo, TimChanNo)				SET_IO_BIT((TimNo + TIM_EGR_OFFSET), (BIT_0 + TimChanNo))
#define TIM_SET_UPDATE_GEN(TimNo)							SET_IO_BIT((TimNo + TIM_EGR_OFFSET), BIT_0)

#define TIM_CCMR_CHAN_ADDR(TimNo, TimChanNo)				(TimNo + (TIM_CCMR1_OFFSET + (((TimChanNo - 1) / 2) * 0x4)))
#define TIM_CCMR_START_BIT(TimChanNo)						(((TimChanNo - 1) % 2) * 8)

#define TIM_OUT_COMP_MOD__FROZEN				0x0
#define TIM_OUT_COMP_MOD__MATCH_HIGH			0x1
#define TIM_OUT_COMP_MOD__MATCH_LOW				0x2
#define TIM_OUT_COMP_MOD__MATCH_TOGGLE			0x3
#define TIM_OUT_COMP_MOD__FORCE_INACTIVE		0x4
#define TIM_OUT_COMP_MOD__FORCE_ACTIVE			0x5
#define TIM_OUT_COMP_MOD__PWM_1					0x6
#define TIM_OUT_COMP_MOD__PWM_2					0x7

#define TIM_SET_CHAN_OUTPUT(TimNo, TimChanNo, EnaFastComp, EnaPreloadComp, OutCompMode, EnaClearComp)		\
						SET_IO_BITS(																		\
								 TIM_CCMR_CHAN_ADDR(TimNo, TimChanNo),										\
								 TIM_CCMR_START_BIT(TimChanNo),												\
								 BITMASK_8,																	\
								 (																			\
									((EnaClearComp & 0x1) << (TIM_CCMR_START_BIT(TimChanNo) + BIT_7)) |		\
									((OutCompMode & 0x7) << (TIM_CCMR_START_BIT(TimChanNo) + BIT_4)) |		\
									((EnaPreloadComp & 0x1) << (TIM_CCMR_START_BIT(TimChanNo) + BIT_3)) |	\
									((EnaFastComp & 0x1) << (TIM_CCMR_START_BIT(TimChanNo) + BIT_2))		\
								 )																			\
								)

#define TIM_INP_CAPT_MOD__MAPPED_ON_TIM1		0x1
#define TIM_INP_CAPT_MOD__MAPPED_ON_TIM2		0x2
#define TIM_INP_CAPT_MOD__MAPPED_ON_TRC			0x3

#define TIM_INP_CAPT_PRESCALE__NONE					0x0
#define TIM_INP_CAPT_PRESCALE__ONCE_EVERY_2_EVT		0x1
#define TIM_INP_CAPT_PRESCALE__ONCE_EVERY_4_EVT		0x2
#define TIM_INP_CAPT_PRESCALE__ONCE_EVERY_8_EVT		0x3

#define TIM_INP_CAPT_FILT__NONE				0x0
#define TIM_INP_CAPT_FILT__1				0x1
#define TIM_INP_CAPT_FILT__2				0x2
#define TIM_INP_CAPT_FILT__3				0x3
#define TIM_INP_CAPT_FILT__4				0x4
#define TIM_INP_CAPT_FILT__5				0x5
#define TIM_INP_CAPT_FILT__6				0x6
#define TIM_INP_CAPT_FILT__7				0x7
#define TIM_INP_CAPT_FILT__8				0x8
#define TIM_INP_CAPT_FILT__9				0x9
#define TIM_INP_CAPT_FILT__10				0xA
#define TIM_INP_CAPT_FILT__11				0xB
#define TIM_INP_CAPT_FILT__12				0xC
#define TIM_INP_CAPT_FILT__13				0xD
#define TIM_INP_CAPT_FILT__14				0xE
#define TIM_INP_CAPT_FILT__15				0xF

#define TIM_SET_CHAN_INPUT(TimNo, TimChanNo, InpCaptMod, InpCaptPrescale, InpCaptFilt)						\
						SET_IO_BITS(																		\
								 TIM_CCMR_CHAN_ADDR(TimNo, TimChanNo),										\
								 TIM_CCMR_START_BIT(TimChanNo),												\
								 BITMASK_8,																	\
								 (																			\
									((InpCaptFilt & 0xF) << (TIM_CCMR_START_BIT(TimChanNo) + BIT_4)) |		\
									((InpCaptPrescale & 0x3) << (TIM_CCMR_START_BIT(TimChanNo) + BIT_2)) |	\
									((InpCaptMod & 0x3) << TIM_CCMR_START_BIT(TimChanNo))					\
								 )																			\
								)

#define TIM_ENA_CHAN(TimNo, TimChanNo, Enable)				\
	ENABLE_IO_BIT((TimNo + TIM_CCER_OFFSET), ((TimChanNo - 1) * 4), Enable)

#define TIM_ENA_LOW_OR_INVERT_POLARITY(TimNo, TimChanNo, Enable)		\
	ENABLE_IO_BIT((TimNo + TIM_CCER_OFFSET), ((TimChanNo * 4) - 3), Enable)

#define TIM_GET_COUNTER(TimNo)								GET_IO_BITS((TimNo + TIM_CNT_OFFSET), BIT_0, BITMASK_16)
#define TIM_SET_COUNTER(TimNo, Val)							SET_IO_BITS((TimNo + TIM_CNT_OFFSET), BIT_0, BITMASK_16, Val)

#define TIM_GET_PRESCALER(TimNo)							GET_IO_BITS((TimNo + TIM_PSC_OFFSET), BIT_0, BITMASK_16)
#define TIM_SET_PRESCALER(TimNo, Val)						SET_IO_BITS((TimNo + TIM_PSC_OFFSET), BIT_0, BITMASK_16, Val)

#define TIM_GET_AUTO_RELOAD(TimNo)							GET_IO_BITS((TimNo + TIM_ARR_OFFSET), BIT_0, BITMASK_16)
#define TIM_SET_AUTO_RELOAD(TimNo, Val)						SET_IO_BITS((TimNo + TIM_ARR_OFFSET), BIT_0, BITMASK_16, Val)

#define TIM_GET_CAPT_COMP_DAT(TimNo, TimChanNo)				\
	GET_IO_BITS((TimNo + (TIM_CCR1_OFFSET + ((TimChanNo - 1) * 0x4))), BIT_0, BITMASK_16)

#define TIM_SET_CAPT_COMP_DAT(TimNo, TimChanNo, Val)		\
	SET_IO_BITS((TimNo + (TIM_CCR1_OFFSET + ((TimChanNo - 1) * 0x4))), BIT_0, BITMASK_16, Val)

#define TIM_SET_DMA_BURST_LEN(TimNo, Len)					SET_IO_BITS((TimNo + TIM_DCR_OFFSET), BIT_8, BITMASK_5, Len)
#define TIM_SET_DMA_BASE_ADDR(TimNo, Addr)					SET_IO_BITS((TimNo + TIM_DCR_OFFSET), BIT_0, BITMASK_5, Addr)

#define TIM_SET_DMA_ADDR_FOR_FULL_TRANS(TimNo, Addr)		SET_IO_BITS((TimNo + TIM_DMAR_OFFSET), BIT_0, BITMASK_16, Addr)

#endif  // End of TIMER_H
