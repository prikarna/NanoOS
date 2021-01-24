/*
 * File    : ADC.h
 * Remark  : Macro to access Analog To Digital Converter controller on chip.
 *           Fow now this is STM32F10XXX chipset.
 *
 */

#ifndef ADC_H
#define ADC_H

#include "..\Type.h"
#include "..\Bit.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "ChipAddr.h"

#define ADC1_BASE				0x40012400
#define ADC2_BASE				0x40012800
#define ADC3_BASE				0x40013C00

#define ADC1					ADC1_BASE
#define ADC2					ADC2_BASE
#define	ADC3					ADC3_BASE

#define ADC_SR_OFFSET			0x00
#define ADC_CR1_OFFSET			0x04
#define ADC_CR2_OFFSET			0x08
#define ADC_SMPR1_OFFSET		0x0C
#define ADC_SMPR2_OFFSET		0x10
#define ADC_JOFR1_OFFSET		0x14
#define ADC_JOFR2_OFFSET		0x18
#define ADC_JOFR3_OFFSET		0x1C
#define ADC_JOFR4_OFFSET		0x20
#define ADC_HTR_OFFSET			0x24
#define ADC_LTR_OFFSET			0x28
#define ADC_SQR1_OFFSET			0x2C
#define ADC_SQR2_OFFSET			0x30
#define ADC_SQR3_OFFSET			0x34
#define ADC_JSQR_OFFSET			0x38
#define ADC_JDR1_OFFSET			0x3C
#define ADC_JDR2_OFFSET			0x40
#define ADC_JDR3_OFFSET			0x44
#define ADC_JDR4_OFFSET			0x48
#define ADC_DR_OFFSET			0x4C

#define ADC_CHAN__0				0
#define ADC_CHAN__1				1
#define ADC_CHAN__2				2
#define ADC_CHAN__3				3
#define ADC_CHAN__4				4
#define ADC_CHAN__5				5
#define ADC_CHAN__6				6
#define ADC_CHAN__7				7
#define ADC_CHAN__8				8
#define ADC_CHAN__9				9
#define ADC_CHAN__10			10
#define ADC_CHAN__11			11
#define ADC_CHAN__12			12
#define ADC_CHAN__13			13
#define ADC_CHAN__14			14
#define ADC_CHAN__15			15
#define ADC_CHAN__16			16
#define ADC_CHAN__17			17
#define ADC_CHAN__INT_TEMP_SENS	ADC_CHAN__16
#define ADC_CHAN__INT_VREFIN	ADC_CHAN__17

#define ADC_ADDR(AdcNo, RegOffset)						(AdcNo + RegOffset)

#define ADC_GET_REG_GRP_START_FLAG(AdcNo)				GET_IO_BIT(ADC_ADDR(AdcNo, ADC_SR_OFFSET), BIT_4)
#define ADC_CLR_REG_GRP_START_FLAG(AdcNo)				CLR_IO_BIT(ADC_ADDR(AdcNo, ADC_SR_OFFSET), BIT_4)

#define ADC_GET_INJ_GRP_START_FLAG(AdcNo)				GET_IO_BIT(ADC_ADDR(AdcNo, ADC_SR_OFFSET), BIT_3)
#define ADC_CLR_INJ_GRP_START_FLAG(AdcNo)				CLR_IO_BIT(ADC_ADDR(AdcNo, ADC_SR_OFFSET), BIT_3)

#define ADC_GET_INJ_GRP_END_OF_CONV(AdcNo)				GET_IO_BIT(ADC_ADDR(AdcNo, ADC_SR_OFFSET), BIT_2)
#define ADC_CLR_INJ_GRP_END_OF_CONV(AdcNo)				CLR_IO_BIT(ADC_ADDR(AdcNo, ADC_SR_OFFSET), BIT_2)

#define ADC_GET_END_OF_CONV(AdcNo)						GET_IO_BIT(ADC_ADDR(AdcNo, ADC_SR_OFFSET), BIT_1)
#define ADC_CLR_END_OF_CONV(AdcNo)						CLR_IO_BIT(ADC_ADDR(AdcNo, ADC_SR_OFFSET), BIT_1)

#define ADC_GET_ANALOG_WATCHDOG_FLAG(AdcNo)				GET_IO_BIT(ADC_ADDR(AdcNo, ADC_SR_OFFSET), BIT_0)
#define ADC_CLR_ANALOG_WATCHDOG_FLAG(AdcNo)				CLR_IO_BIT(ADC_ADDR(AdcNo, ADC_SR_OFFSET), BIT_0)

#define ADC_CLR_ALL_CONV_FLAGS(AdcNo)					SET_IO_BITS(ADC_ADDR(AdcNo, ADC_SR_OFFSET), BIT_0, BITMASK_4, 0)

#define ADC_DUAL_MODE__INDEPENDENT								0x00
#define ADC_DUAL_MODE__REG_SIMULTANEOUS_AND_INJ_SIMULTANEOUS	0x01
#define ADC_DUAL_MODE__REG_SIMULTANEOUS_AND_ALT_TRIGGER			0x02
#define ADC_DUAL_MODE__INJ_SIMULTANEOUS_AND_FAST_INTERLEAVED	0x03
#define ADC_DUAL_MODE__INJ_SIMULTANEOUS_AND_SLOW_INTERLEAVED	0x04
#define ADC_DUAL_MODE__INJ_SIMULTANEOUS_ONLY					0x05
#define ADC_DUAL_MODE__REG_SIMULTANEOUS_ONLY					0x06
#define ADC_DUAL_MODE__FAST_INTERLEAVED_ONLY					0x07
#define ADC_DUAL_MODE__SLOW_INTERLEAVED_ONLY					0x08
#define ADC_DUAL_MODE__ALT_TRIGGER_ONLY							0x09

#define ADC_SET_DUAL_MODE_SEL(AdcNo, AdcMode)			SET_IO_BITS(ADC_ADDR(AdcNo, ADC_CR1_OFFSET), BIT_16, BITMASK_4, AdcMode)

#define ADC_DISC_COUNT__1_CHAN			0x00
#define ADC_DISC_COUNT__2_CHANS			0x01
#define ADC_DISC_COUNT__3_CHANS			0x02
#define ADC_DISC_COUNT__4_CHANS			0x03
#define ADC_DISC_COUNT__5_CHANS			0x04
#define ADC_DISC_COUNT__6_CHANS			0x05
#define ADC_DISC_COUNT__7_CHANS			0x06
#define ADC_DISC_COUNT__8_CHANS			0x07

#define ADC_SET_DISC_CHAN_COUNT(AdcNo, AdcDiscCount)	SET_IO_BITS(ADC_ADDR(AdcNo, ADC_CR1_OFFSET), BIT_13, BITMASK_3, AdcDiscCount)

#define ADC_ENA_INJ_GRP_DISC_MOD(AdcNo, Ena)			ENABLE_IO_BIT(ADC_ADDR(AdcNo, ADC_CR1_OFFSET), BIT_12, Ena)

#define ADC_ENA_REG_GRP_DISC_MOD(AdcNo, Ena)			ENABLE_IO_BIT(ADC_ADDR(AdcNo, ADC_CR1_OFFSET), BIT_11, Ena)

#define ADC_ENA_INJ_GRP_AUTO_CONV(AdcNo, Ena)			ENABLE_IO_BIT(ADC_ADDR(AdcNo, ADC_CR1_OFFSET), BIT_10, Ena)

#define ADC_ENA_SCAN_MOD(AdcNo, Ena)					ENABLE_IO_BIT(ADC_ADDR(AdcNo, ADC_CR1_OFFSET), BIT_8, Ena)

#define ADC_ENA_INJ_GRP_INT(AdcNo, Ena)					ENABLE_IO_BIT(ADC_ADDR(AdcNo, ADC_CR1_OFFSET), BIT_7, Ena)

#define ADC_ENA_END_OF_CONV_INT(AdcNo, Ena)				ENABLE_IO_BIT(ADC_ADDR(AdcNo, ADC_CR1_OFFSET), BIT_5, Ena)

#define ADC_ENA_TEMP_SENS_AND_VREFIN(Ena)				ENABLE_IO_BIT((ADC1_BASE + ADC_CR2_OFFSET), BIT_23, Ena)

#define ADC_START_REG_GRP_CONV(AdcNo)					SET_IO_BIT(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_22)
#define ADC_GET_REG_GRP_START_CONV_STATUS(AdcNo)		GET_IO_BIT(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_22)

#define ADC_START_INJ_GRP_CONV(AdcNo)					SET_IO_BIT(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_21)

#define ADC_ENA_REG_GRP_EXT_TRIG(AdcNo, Ena)			ENABLE_IO_BIT(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_20, Ena)

#define ADC12_REG_GRP_EXT_TRIG__TIMER1_CC1		0x00	// 000: Timer 1 CC1 event
#define ADC12_REG_GRP_EXT_TRIG__TIMER1_CC2		0x01	// 001: Timer 1 CC2 event
#define ADC12_REG_GRP_EXT_TRIG__TIMER1_CC3		0x02	// 010: Timer 1 CC3 event
#define ADC12_REG_GRP_EXT_TRIG__TIMER2_CC2		0x03	// 011: Timer 2 CC2 event
#define ADC12_REG_GRP_EXT_TRIG__TIMER3_TRGO		0x04	// 100: Timer 3 TRGO event
#define ADC12_REG_GRP_EXT_TRIG__TIMER4_CC4		0x05	// 101: Timer 4 CC4 event
#define ADC12_REG_GRP_EXT_TRIG__EXT_LINE11		0x06	// 110: EXTI line 11/TIM8_TRGO event
#define ADC12_REG_GRP_EXT_TRIG__SWSTART			0x07	// 111: SWSTART

#define ADC12_SET_REG_GRP_EXT_TRIG(AdcNo, Adc12RegGrpExtTrig)					\
	SET_IO_BITS(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_17, BITMASK_3, Adc12RegGrpExtTrig)

#define ADC3_REG_GRP_EXT_TRIG__TIMER3_CC1		0x00	// 000: Timer 3 CC1 event
#define ADC3_REG_GRP_EXT_TRIG__TIMER2_CC3		0x01	// 001: Timer 2 CC3 event
#define ADC3_REG_GRP_EXT_TRIG__TIMER1_CC3		0x02	// 010: Timer 1 CC3 event
#define ADC3_REG_GRP_EXT_TRIG__TIMER8_CC1		0x03	// 011: Timer 8 CC1 event
#define ADC3_REG_GRP_EXT_TRIG__TIMER8_TRGO		0x04	// 100: Timer 8 TRGO event
#define ADC3_REG_GRP_EXT_TRIG__TIMER5_CC1		0x05	// 101: Timer 5 CC1 event
#define ADC3_REG_GRP_EXT_TRIG__TIMER5_CC3		0x06	// 110: Timer 5 CC3 event
#define ADC3_REG_GRP_EXT_TRIG__SWSTART			0x07	// 111: SWSTART

#define ADC3_SET_REG_GRP_EXT_TRIG(Adc3RegGrpExtTrig)							\
	SET_IO_BITS(ADC_ADDR(ADC3_BASE + ADC_CR2_OFFSET), BIT_17, BITMASK_3, Adc3RegGrpExtTrig)

#define ADC_ENA_INJ_GRP_EXT_TRIG(AdcNo, Ena)			ENABLE_IO_BIT(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_15, Ena)

#define ADC12_INJ_GRP_EXT_TRIG__TIMER1_TRGO		0x00	// 000: Timer 1 TRGO event
#define ADC12_INJ_GRP_EXT_TRIG__TIMER1_CC4		0x01	// 001: Timer 1 CC4 event
#define ADC12_INJ_GRP_EXT_TRIG__TIMER2_TRGO		0x02	// 010: Timer 2 TRGO event
#define ADC12_INJ_GRP_EXT_TRIG__TIMER2_CC1		0x03	// 011: Timer 2 CC1 event
#define ADC12_INJ_GRP_EXT_TRIG__TIMER3_CC4		0x04	// 100: Timer 3 CC4 event
#define ADC12_INJ_GRP_EXT_TRIG__TIMER4_TRGO		0x05	// 101: Timer 4 TRGO event
#define ADC12_INJ_GRP_EXT_TRIG__EXTI_LINE15		0x06	// 110: EXTI line15/TIM8_CC4 event
#define ADC12_INJ_GRP_EXT_TRIG__JSWSTART		0x07	// 111: JSWSTART

#define ADC12_SET_INJ_GRP_EXT_TRIG(AdcNo, Adc12InjExtTrig)					\
	SET_IO_BITS(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_12, BITMASK_3, Adc12InjExtTrig)

#define ADC3_INJ_GRP_EXT_TRIG__TIMER1_TRGO		0x00	// 000: Timer 1 TRGO event
#define ADC3_INJ_GRP_EXT_TRIG__TIMER1_CC4		0x01	// 001: Timer 1 CC4 event
#define ADC3_INJ_GRP_EXT_TRIG__TIMER4_CC3		0x02	// 010: Timer 4 CC3 event
#define ADC3_INJ_GRP_EXT_TRIG__TIMER8_CC2		0x03	// 011: Timer 8 CC2 event
#define ADC3_INJ_GRP_EXT_TRIG__TIMER8_CC4		0x04	// 100: Timer 8 CC4 event
#define ADC3_INJ_GRP_EXT_TRIG__TIMER5_TRGO		0x05	// 101: Timer 5 TRGO event
#define ADC3_INJ_GRP_EXT_TRIG__TIMER5_CC4		0x06	// 110: Timer 5 CC4 event
#define ADC3_INJ_GRP_EXT_TRIG__JSWSTART			0x07	// 111: JSWSTART

#define ADC3_SET_INJ_GRP_EXT_TRIG(Adc3InjGrpExtTrig)						\
	SET_IO_BITS((ADC3_BASE + ADC_CR2_OFFSET), BIT_12, BITMASK_3, Adc3InjGrpExtTrig)

#define ADC_ENA_LEFT_ALIGN_DAT(AdcNo, Ena)			ENABLE_IO_BIT(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_11, Ena)

#define ADC_ENA_DMA(AdcNo, Ena)						ENABLE_IO_BIT(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_8, Ena)

#define ADC_INIT_CALIB(AdcNo)						SET_IO_BIT(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_3)
#define ADC_IS_INIT_CALIB_COMPLETE(AdcNo)			(GET_IO_BIT(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_3)) ? 0 : 1

#define ADC_CALIBRATE(AdcNo)						SET_IO_BIT(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_2)
#define ADC_IS_CALIB_DONE(AdcNo)					(GET_IO_BIT(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_2)) ? 0 : 1

#define ADC_ENA_CONT_MOD(AdcNo, Ena)				ENABLE_IO_BIT(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_1, Ena)

#define ADC_ENABLE(AdcNo, Ena)						ENABLE_IO_BIT(ADC_ADDR(AdcNo, ADC_CR2_OFFSET), BIT_0, Ena)

#define ADC_SAMPLE__1_PT_5_CYCLES			0x00
#define ADC_SAMPLE__7_PT_5_CYCLES			0x01
#define ADC_SAMPLE__13_PT_5_CYCLES			0x02
#define ADC_SAMPLE__28_PT_5_CYCLES			0x03
#define ADC_SAMPLE__41_PT_5_CYCLES			0x04
#define ADC_SAMPLE__55_PT_5_CYCLES			0x05
#define ADC_SAMPLE__71_PT_5_CYCLES			0x06
#define ADC_SAMPLE__239_PT_5_CYCLES			0x07

#define ADC_SAMPL_REG(AdcNo, AdcChan)		((AdcNo + ADC_SMPR2_OFFSET) - ((AdcChan / 10) * 0x04))

#define ADC_SAMPL_NTH_PIN(AdcChan)			((AdcChan * 3) - ((AdcChan / 10) * 30))

#define ADC_SET_SAMPLE(AdcNo, AdcChan, AdcSample)		\
	SET_IO_BITS(ADC_SAMPL_REG(AdcNo, AdcChan), ADC_SAMPL_NTH_PIN(AdcChan), BITMASK_3, AdcSample)

#define ADC_SET_INJ_GRP_DATA_OFFSET(AdcNo, AdcChan, DatOffset)						\
	IO_MEM32(ADC_ADDR(AdcNo, (ADC_JOFR1_OFFSET + (AdcChan * 0x4)))) =				\
	(IO_MEM32(ADC_ADDR(AdcNo, (ADC_JOFR1_OFFSET + (AdcChan * 0x4)))) &= ~(0xFFF)) |	\
	(DatOffset & 0xFFF)

#define ADC_REG_GRP_SEQ_LEN__1						0x00
#define ADC_REG_GRP_SEQ_LEN__2						0x01
#define ADC_REG_GRP_SEQ_LEN__3						0x02
#define ADC_REG_GRP_SEQ_LEN__4						0x03
#define ADC_REG_GRP_SEQ_LEN__5						0x04
#define ADC_REG_GRP_SEQ_LEN__6						0x05
#define ADC_REG_GRP_SEQ_LEN__7						0x06
#define ADC_REG_GRP_SEQ_LEN__8						0x07
#define ADC_REG_GRP_SEQ_LEN__9						0x08
#define ADC_REG_GRP_SEQ_LEN__10						0x09
#define ADC_REG_GRP_SEQ_LEN__11						0x0A
#define ADC_REG_GRP_SEQ_LEN__12						0x0B
#define ADC_REG_GRP_SEQ_LEN__13						0x0C
#define ADC_REG_GRP_SEQ_LEN__14						0x0D
#define ADC_REG_GRP_SEQ_LEN__15						0x0E
#define ADC_REG_GRP_SEQ_LEN__16						0x0F

#define ADC_SET_REG_GRP_SEQ_LEN(AdcNo, AdcRegGrpSeqLen)			\
	SET_IO_BITS(ADC_ADDR(AdcNo, ADC_SQR1_OFFSET), BIT_20, BITMASK_4, AdcRegGrpSeqLen)

#define ADC_REG_GRP_CHAN_SEQ__1ST					0x00
#define ADC_REG_GRP_CHAN_SEQ__2ND					0x01
#define ADC_REG_GRP_CHAN_SEQ__3TH					0x02
#define ADC_REG_GRP_CHAN_SEQ__4TH					0x03
#define ADC_REG_GRP_CHAN_SEQ__5TH					0x04
#define ADC_REG_GRP_CHAN_SEQ__6TH					0x05
#define ADC_REG_GRP_CHAN_SEQ__7TH					0x06
#define ADC_REG_GRP_CHAN_SEQ__8TH					0x07
#define ADC_REG_GRP_CHAN_SEQ__9TH					0x08
#define ADC_REG_GRP_CHAN_SEQ__10TH					0x09
#define ADC_REG_GRP_CHAN_SEQ__11TH					0x0A
#define ADC_REG_GRP_CHAN_SEQ__12TH					0x0B
#define ADC_REG_GRP_CHAN_SEQ__13TH					0x0C
#define ADC_REG_GRP_CHAN_SEQ__14TH					0x0D
#define ADC_REG_GRP_CHAN_SEQ__15TH					0x0E
#define ADC_REG_GRP_CHAN_SEQ__16TH					0x0F

#define ADC_REG_GRP_SEQ_REG(AdcNo, AdcRegGrpSeq)			((AdcNo + ADC_SQR3_OFFSET) - ((AdcRegGrpSeq / 6) * 0x04))

#define ADC_REG_GRP_SEQ_NTH_BIT(AdcRegGrpSeq)				((AdcRegGrpSeq * 5) - ((AdcRegGrpSeq / 6) * 30))

#define ADC_SET_REG_GRP_CHAN_SEQ(AdcNo, AdcChan, AdcRegGrpChanSeq)		\
	SET_IO_BITS(ADC_REG_GRP_SEQ_REG(AdcNo, AdcRegGrpChanSeq), ADC_REG_GRP_SEQ_NTH_BIT(AdcRegGrpChanSeq), BITMASK_5, AdcChan)

#define ADC_INJ_GRP_SEQ_LEN__1						0x00
#define ADC_INJ_GRP_SEQ_LEN__2						0x01
#define ADC_INJ_GRP_SEQ_LEN__3						0x02
#define ADC_INJ_GRP_SEQ_LEN__4						0x03

#define ADC_SET_INJ_GRP_SEQ_LEN(AdcNo, AdcInjGrpSeqLen)				\
	SET_IO_BITS(ADC_ADDR(AdcNo, ADC_JSQR_OFFSET), BIT_20, BITMASK_2, AdcInjGrpSeqLen)

#define ADC_INJ_GRP_CHAN_SEQ__1ST						0x00
#define ADC_INJ_GRP_CHAN_SEQ__2ND						0x01
#define ADC_INJ_GRP_CHAN_SEQ__3TH						0x02
#define ADC_INJ_GRP_CHAN_SEQ__4TH						0x03

#define ADC_SET_INJ_GRP_CHAN_SEQ(AdcNo, AdcChan, AdcInjGrpChanSeq)		\
	SET_IO_BITS(ADC_ADDR(AdcNo, ADC_JSQR_OFFSET), (AdcChan * 0x4), BITMASK_5, AdcInjGrpChanSeq)

#define ADC_GET_INJ_GRP_DATA(AdcNo, AdcChan)						\
	GET_IO_BITS(ADC_ADDR(AdcNo, (ADC_JDR1_OFFSET + (AdcChan * 0x4))), 0, BITMASK_16)

#define ADC_GET_REG_GRP_DATA(AdcNo)									\
	GET_IO_BITS(ADC_ADDR(AdcNo, ADC_DR_OFFSET), 0, BITMASK_16)

#ifdef __cplusplus
}
#endif

#endif  // End of ADC_H
