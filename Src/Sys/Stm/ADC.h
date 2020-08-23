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

#define ADC_REG_CHAN_START_FLAG				BITHEX_4
#define ADC_INJ_CHAN_START_FLAG				BITHEX_3
#define ADC_INJ_CHAN_END_OF_CONV			BITHEX_2
#define ADC_END_OF_CONV						BITHEX_1
#define ADC_ANALOG_WATCHDOG_FLAG			BITHEX_0

#define ADC_GET_REG_CHAN_START_FL(AdcNo)		\
	(IO_MEM32((AdcNo + ADC_SR_OFFSET)) & ADC_REG_CHAN_START_FLAG) ? 1 : 0

#define ADC_CLR_REG_CHAN_START_FL(AdcNo)		\
	IO_MEM32((AdcNo + ADC_SR_OFFSET)) &= ~ADC_REG_CHAN_START_FLAG

#define ADC_GET_INJ_CHAN_START_FL(AdcNo)		\
	(IO_MEM32((AdcNo + ADC_SR_OFFSET)) & ADC_INJ_CHAN_START_FLAG) ? 1 : 0

#define ADC_CLR_INJ_CHAN_START_FL(AdcNo)		\
	IO_MEM32((AdcNo + ADC_SR_OFFSET)) &= ~ADC_INJ_CHAN_START_FLAG

#define ADC_GET_INJ_CHAN_END_OF_CONV(AdcNo)		\
	(IO_MEM32((AdcNo + ADC_SR_OFFSET)) & ADC_INJ_CHAN_END_OF_CONV) ? 1 : 0

#define ADC_CLR_INJ_CHAN_END_OF_CONV(AdcNo)		\
	IO_MEM32((AdcNo + ADC_SR_OFFSET)) &= ~ADC_INJ_CHAN_END_OF_CONV

#define ADC_GET_END_OF_CONV(AdcNo)				\
	(IO_MEM32((AdcNo + ADC_SR_OFFSET)) & ADC_END_OF_CONV) ? 1 : 0

#define ADC_CLR_END_OF_CONV(AdcNo)				\
	IO_MEM32((AdcNo + ADC_SR_OFFSET)) &= ~ADC_END_OF_CONV

#define ADC_GET_ANALOG_WATCHDOG_FL(AdcNo)		\
	(IO_MEM32((AdcNo + ADC_SR_OFFSET)) & ADC_ANALOG_WATCHDOG_FLAG) ? 1 : 0

#define ADC_CLR_ANALOG_WATCHDOG_FL(AdcNo)		\
	IO_MEM32((AdcNo + ADC_SR_OFFSET)) &= ~ADC_ANALOG_WATCHDOG_FLAG

#define ADC_ANALOG_WATCHDOG_ON_REG_CHAN_EN					BITHEX_23
#define ADC_ANALOG_WATCHDOG_ON_INJ_CHAN_EN					BITHEX_22
#define ADC_DISC_MOD_ON_INJ_CHAN_EN							BITHEX_12
#define ADC_DISC_MOD_ON_REG_CHAN_EN							BITHEX_11
#define ADC_AUTO_INJ_GRP_CONV_EN							BITHEX_10
#define ADC_WATCHDOG_ON_SINGLE_CHAN_EN						BITHEX_9
#define ADC_SCAN_MODE_EN									BITHEX_8
#define ADC_INJ_CHAN_INT_EN									BITHEX_7
#define ADC_ANALOG_WATCHDOG_INT_EN							BITHEX_6
#define ADC_END_OF_CONV_INT_EN								BITHEX_5

#define ADC_ENA_ANALOG_WATCHDOG_ON_REG_CHAN(AdcNo, Ena)								\
	(Ena) ?																			\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) |= ADC_ANALOG_WATCHDOG_ON_REG_CHAN_EN) :		\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) &= ~ADC_ANALOG_WATCHDOG_ON_REG_CHAN_EN)

#define ADC_ENA_ANALOG_WATCHDOG_ON_INJ_CHAN(AdcNo, Ena)								\
	(Ena) ?																			\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) |= ADC_ANALOG_WATCHDOG_ON_INJ_CHAN_EN) :		\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) &= ~ADC_ANALOG_WATCHDOG_ON_INJ_CHAN_EN)

#define ADC_MODE__INDEPENDENT								0x00
#define ADC_MODE__REG_SIMULTANEOUS_AND_INJ_SIMULTANEOUS		0x01
#define ADC_MODE__REG_SIMULTANEOUS_AND_ALT_TRIGGER			0x02
#define ADC_MODE__INJ_SIMULTANEOUS_AND_FAST_INTERLEAVED		0x03
#define ADC_MODE__INJ_SIMULTANEOUS_AND_SLOW_INTERLEAVED		0x04
#define ADC_MODE__INJ_SIMULTANEOUS_ONLY						0x05
#define ADC_MODE__REG_SIMULTANEOUS_ONLY						0x06
#define ADC_MODE__FAST_INTERLEAVED_ONLY						0x07
#define ADC_MODE__SLOW_INTERLEAVED_ONLY						0x08
#define ADC_MODE__ALT_TRIGGER_ONLY							0x09

#define ADC_SET_DUAL_MODE(AdcNo, AdcMode)					SET_IO_BITS((AdcNo + ADC_CR1_OFFSET), 16, BITMASK_4, AdcMode)

#define ADC_COUNT__1_CHAN			0x00
#define ADC_COUNT__2_CHANS			0x01
#define ADC_COUNT__3_CHANS			0x02
#define ADC_COUNT__4_CHANS			0x03
#define ADC_COUNT__5_CHANS			0x04
#define ADC_COUNT__6_CHANS			0x05
#define ADC_COUNT__7_CHANS			0x06
#define ADC_COUNT__8_CHANS			0x07

#define ADC_SET_DISC_CHAN_COUNT(AdcNo, AdcCount)		SET_IO_BITS((AdcNo + ADC_CR1_OFFSET), 13, BITMASK_3, AdcCount)

#define ADC_ENA_DISC_MODE_ON_INJ_CHAN(AdcNo, Ena)							\
	(Ena) ?																	\
	(IO_MEM32((AdcMo + ADC_CR1_OFFSET)) |= ADC_DISC_MOD_ON_INJ_CHAN_EN) :	\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) &= ~ ADC_DISC_MOD_ON_INJ_CHAN_EN)

#define ADC_ENA_DISC_MODE_ON_REG_CHAN(AdcNo, Ena)							\
	(Ena) ?																	\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) |= ADC_DISC_MOD_ON_REG_CHAN_EN) :	\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) &= ~ADC_DISC_MOD_ON_INJ_CHAN_EN)

#define ADC_ENA_AUTO_INJ_GRP_CONV(AdcNo, Ena)								\
	(Ena) ?																	\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) |= ADC_AUTO_INJ_GRP_CONV_EN) :		\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) &= ~ADC_AUTO_INJ_GRP_CONV_EN)

#define ADC_ENA_WATCHDOG_ON_SINGLE_CHAN_IN_SCAN_MODE(AdcNo, Ena)			\
	(Ena) ?																	\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) |= ADC_WATCHDOG_ON_SINGLE_CHAN_EN) :	\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) &= ~ADC_WATCHDOG_ON_SINGLE_CHAN_EN)

#define ADC_ENA_SCAN_MODE(AdcNo, Ena)										\
	(Ena) ?																	\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) |= ADC_SCAN_MODE_EN) :				\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) &= ~ADC_SCAN_MODE_EN)

#define ADC_ENA_INJ_CHAN_INT(AdcNo, Ena)									\
	(Ena) ?																	\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) |= ADC_INJ_CHAN_INT_EN) :			\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) &= ~ADC_INJ_CHAN_INT_EN)

#define ADC_ENA_ANALOG_WATCHDOG_INT(AdcNo, Ena)								\
	(Ena) ?																	\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) |= ADC_ANALOG_WATCHDOG_INT_EN) :	\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) &= ~ADC_ANALOG_WATCHDOG_INT_EN)

#define ADC_CHAN__0					0x00
#define ADC_CHAN__1					0x01
#define ADC_CHAN__2					0x02
#define ADC_CHAN__3					0x03
#define ADC_CHAN__4					0x04
#define ADC_CHAN__5					0x05
#define ADC_CHAN__6					0x06
#define ADC_CHAN__7					0x07
#define ADC_CHAN__8					0x08
#define ADC_CHAN__9					0x09
#define ADC_CHAN__10				0x0A
#define ADC_CHAN__11				0x0B
#define ADC_CHAN__12				0x0C
#define ADC_CHAN__13				0x0E
#define ADC_CHAN__14				0x0F
#define ADC_CHAN__15				0x10
#define ADC_CHAN__16				0x11
#define ADC_CHAN__17				0x12

#define ADC_SELECT_WATCHDOG_CHAN(AdcNo, AdcChan)							\
	IO_MEM32((AdcNo + ADC_CR1_OFFSET)) =									\
	(IO_MEM32((AdcNo + ADC_CR1_OFFSET)) &= ~(0x1F)) | (AdcChan & 0x1F)

#define ADC_ENA_TEMP_SENS_AND_VERIFINT(Ena)									\
	(Ena) ?																	\
	(IO_MEM32((ADC1_BASE + ADC_CR2_OFFSET)) |= (BITHEX_23 | BITHEX_0)) :	\
	(IO_MEM32((ADC1_BASE + ADC_CR2_OFFSET)) &= ~(BITHEX_23 | BITHEX_0))

#define ADC_START_REG_CHAN_CONV(AdcNo)			IO_MEM32((AdcNo + ADC_CR2_OFFSET)) |= BITHEX_22

#define ADC_START_INJ_CHAN_CONV(AdcNo)			IO_MEM32((AdcNo + ADC_CR2_OFFSET)) |= BITHEX_21

#define ADC_ENA_EXT_TRIG_ON_REG_CHAN(AdcNo, Ena)							\
	(Ena) ?																	\
	(IO_MEM32((AdcNo + ADC_CR2_OFFSET)) |= BITHEX_20) :						\
	(IO_MEM32((AdcNo + ADC_CR2_OFFSET)) &= ~BITHEX_20)

#define ADC12_REG_CHAN_EXT_TRIG__TMR1_CC1		0x00	//	000: Timer 1 CC1 event
#define ADC12_REG_CHAN_EXT_TRIG__TMR1_CC2		0x01	//	001: Timer 1 CC2 event
#define ADC12_REG_CHAN_EXT_TRIG__TMR1_CC3		0x02	//	010: Timer 1 CC3 event
#define ADC12_REG_CHAN_EXT_TRIG__TMR2_CC2		0x03	//	011: Timer 2 CC2 event
#define ADC12_REG_CHAN_EXT_TRIG__TMR3_TRGO		0x04	//	100: Timer 3 TRGO event
#define ADC12_REG_CHAN_EXT_TRIG__TMR4_CC4		0x05	//	101: Timer 4 CC4 event
#define ADC12_REG_CHAN_EXT_TRIG__EXTI_LINE_11	0x06	//	110: EXTI line 11/TIM8_TRGO event (TIM8_TRGO is available only in high-density and XLdensity devices)
#define ADC12_REG_CHAN_EXT_TRIG__SWSTART		0x07	//	111: SWSTART

#define ADC12_SET_REG_CHAN_EXT_TRIGGER(AdcNo, Adc12RegExtTrig)		\
	IO_MEM32((AdcNo + ADC_CR2_OFFSET)) = (IO_MEM32((AdcNo + ADC_CR2_OFFSET)) & ~(0x7 << 17)) | ((Adc12RegExtTrig & 0x7) << 17)

#define ADC3_REG_CHAN_EXT_TRIG__TMR3_CC1		0x00	//	000: Timer 3 CC1 event
#define ADC3_REG_CHAN_EXT_TRIG__TMR2_CC3		0x01	//	001: Timer 2 CC3 event
#define ADC3_REG_CHAN_EXT_TRIG__TMR1_CC3		0x02	//	010: Timer 1 CC3 event
#define ADC3_REG_CHAN_EXT_TRIG__TMR8_CC1		0x03	//	011: Timer 8 CC1 event
#define ADC3_REG_CHAN_EXT_TRIG__TMR8_TRGO		0x04	//	100: Timer 8 TRGO event
#define ADC3_REG_CHAN_EXT_TRIG__TMR5_CC1		0x05	//	101: Timer 5 CC1 event
#define ADC3_REG_CHAN_EXT_TRIG__TMR5_CC3		0x06	//	110: Timer 5 CC3 event
#define ADC3_REG_CHAN_EXT_TRIG__SWSTART			0x07	//	111: SWSTART

#define ADC3_SET_REG_CHAN_EXT_TRIGGER(Adc3RegExtTrig)		\
	IO_MEM32((ADC3 + ADC_CR2_OFFSET)) = (IO_MEM32((ADC3 + ADC_CR2_OFFSET)) & ~(0x7 << 17)) | ((Adc3RegExtTrig & 0x7) << 17)

#define ADC_ENA_EXT_TRIG_ON_INJ_CHAN(AdcNo, Ena)			\
	(Ena) ?													\
	(IO_MEM32((AdcNo + ADC_CR2_OFFSET)) |= BITHEX_15) :		\
	(IO_MEM32((AdcNo + ADC_CR2_OFFSET)) &= ~BITHEX_15)

#define ADC12_INJ_CHAN_EXT_TRIG__TMR1_TRGO		0x00	//	000: Timer 1 TRGO event
#define ADC12_INJ_CHAN_EXT_TRIG__TMR1_CC4		0x01	//	001: Timer 1 CC4 event
#define ADC12_INJ_CHAN_EXT_TRIG__TMR2_TRGO		0x02	//	010: Timer 2 TRGO event
#define ADC12_INJ_CHAN_EXT_TRIG__TMR2_CC1		0x03	//	011: Timer 2 CC1 event
#define ADC12_INJ_CHAN_EXT_TRIG__TMR3_CC4		0x04	//	100: Timer 3 CC4 event
#define ADC12_INJ_CHAN_EXT_TRIG__TMR4_TRGO		0x05	//	101: Timer 4 TRGO event
#define ADC12_INJ_CHAN_EXT_TRIG__EXTI_LINE_15	0x06	//	110: EXTI line15/TIM8_CC4 event (TIM8_CC4 is available only in high-density and XLdensity devices)
#define ADC12_INJ_CHAN_EXT_TRIG__SWSTART		0x07	//	111: JSWSTART

#define ADC12_SET_INJ_CHAN_EXT_TRIGGER(AdcNo, Adc12InjExtTrig)		\
	IO_MEM32((AdcNo + ADC_CR2_OFFSET)) = (IO_MMAP((AdcNo + ADC_CR2_OFFSET)) & ~(0x7 << 12)) | ((Adc12InjExtTrig & 0x7) << 12)

#define ADC3_INJ_CHAN_EXT_TRIG__TMR1_TRGO		0x00	//	000: Timer 1 TRGO event
#define ADC3_INJ_CHAN_EXT_TRIG__TMR1_CC4		0x01	//	001: Timer 1 CC4 event
#define ADC3_INJ_CHAN_EXT_TRIG__TMR4_CC3		0x02	//	010: Timer 4 CC3 event
#define ADC3_INJ_CHAN_EXT_TRIG__TMR8_CC2		0x03	//	011: Timer 8 CC2 event
#define ADC3_INJ_CHAN_EXT_TRIG__TMR8_CC4		0x04	//	100: Timer 8 CC4 event
#define ADC3_INJ_CHAN_EXT_TRIG__TMR5_TRGO		0x05	//	101: Timer 5 TRGO event
#define ADC3_INJ_CHAN_EXT_TRIG__TMR5_CC4		0x06	//	110: Timer 5 CC4 event
#define ADC3_INJ_CHAN_EXT_TRIG__SWSTART			0x07	//	111: JSWSTART

#define ADC3_SET_INJ_CHAN_EXT_TRIGGER(Adc3InjExtTrig)		\
	IO_MEM32((ADC3 + ADC_CR2_OFFSET)) = (IO_MMAP((ADC3 + ADC_CR2_OFFSET)) & ~(0x7 << 12)) | ((Adc3InjExtTrig & 0x7) << 12)

#define ADC_DAT_ALIGN_RIGHT				0x00
#define ADC_DAT_ALIGN_LEFT				0x01

#define ADC_SET_DATA_ALIGN(AdcNo, AdcDatAlign)								\
	(AdcDatAlign) ?															\
	(IO_MEM32((AdcNo + ADC_CR2_OFFSET)) |= BITHEX_11) :						\
	(IO_MEM32((AdcNo + ADC_CR2_OFFSET)) &= ~BITHEX_11)

#define ADC_ENA_DMA(AdcNo, Ena)												\
	(Ena) ?																	\
	(IO_MEM32((AdcNo + ADC_CR2_OFFSET)) |= BITHEX_8) :						\
	(IO_MEM32((AdcNo + ADC_CR2_OFFSET)) &= ~BITHEX_8)

#define ADC_RESET_CALIB(AdcNo)				IO_MEM32((AdcNo + ADC_CR2_OFFSET)) |= BITHEX_3

#define ADC_ENA_CALIB(AdcNo)				IO_MEM32((AdcNo + ADC_CR2_OFFSET)) |= BITHEX_2

#define ADC_GET_CALIB_STATUS(AdcNo)			(IO_MEM32((AdcNo + ADC_CR2_OFFSET)) & BITHEX_2) ? 1 : 0

#define ADC_ENABLE(AdcNo)					IO_MEM32((AdcNo + ADC_CR2_OFFSET)) |= BITHEX_0

#define ADC_DISABLE(AdcNo)					IO_MEM32((AdcNo + ADC_CR2_OFFSET)) &= ~BITHEX_0

#define ADC_SAMPL__1_PT_5_CYCLES			0x00
#define ADC_SAMPL__7_PT_5_CYCLES			0x01
#define ADC_SAMPL__13_PT_5_CYCLES			0x02
#define ADC_SAMPL__28_PT_5_CYCLES			0x03
#define ADC_SAMPL__41_PT_5_CYCLES			0x04
#define ADC_SAMPL__55_PT_5_CYCLES			0x05
#define ADC_SAMPL__71_PT_5_CYCLES			0x06
#define ADC_SAMPL__239_PT_5_CYCLES			0x07

/*
#define ADC_SET_SAMPLE(AdcNo, AdcChan, AdcSampl)										\
	if ((AdcChan >= ADC_CHAN__0) && (AdcChan <= ADC_CHAN__9))								\
	{																					\
	SET_IO_BITS((AdcNo + ADC_SMPR2_OFFSET), (AdcChan * 3), BITMASK_3, AdcSampl);		\
	}																					\
	else if ((AdcChan >= ADC_CHAN__10) && (AdcChan <= ADC_CHAN__17))						\
	{																					\
	SET_IO_BITS((AdcNo + ADC_SMPR1_OFFSET), ((AdcChan - 10) * 3), BITMASK_3, AdcSampl);	\
	}																					\
	else																				\
	{																					\
	}
	*/

#define ADC_SEQ_LEN__1						0x00
#define ADC_SEQ_LEN__2						0x01
#define ADC_SEQ_LEN__3						0x02
#define ADC_SEQ_LEN__4						0x03
#define ADC_SEQ_LEN__5						0x04
#define ADC_SEQ_LEN__6						0x05
#define ADC_SEQ_LEN__7						0x06
#define ADC_SEQ_LEN__8						0x07
#define ADC_SEQ_LEN__9						0x08
#define ADC_SEQ_LEN__10						0x09
#define ADC_SEQ_LEN__11						0x0A
#define ADC_SEQ_LEN__12						0x0B
#define ADC_SEQ_LEN__13						0x0C
#define ADC_SEQ_LEN__14						0x0D
#define ADC_SEQ_LEN__15						0x0E
#define ADC_SEQ_LEN__16						0x0F

#define ADC_SET_SEQ_LEN(AdcNo, AdcSeqLen)		SET_IO_BITS((AdcNo + ADC_SQR1_OFFSET), 20, BITMASK_4, AdcSeqLen)

#define ADC_SEQ__1ST			0x00
#define ADC_SEQ__2ND			0x01
#define ADC_SEQ__3TH			0x02
#define ADC_SEQ__4TH			0x03
#define ADC_SEQ__5TH			0x04
#define ADC_SEQ__6TH			0x05
#define ADC_SEQ__7TH			0x06
#define ADC_SEQ__8TH			0x07
#define ADC_SEQ__9TH			0x08
#define ADC_SEQ__10TH			0x09
#define ADC_SEQ__11TH			0x0A
#define ADC_SEQ__12TH			0x0B
#define ADC_SEQ__13TH			0x0C
#define ADC_SEQ__14TH			0x0D
#define ADC_SEQ__15TH			0x0E
#define ADC_SEQ__16TH			0x0F

/*
#define ADC_SET_SEQ(AdcNo, AdcChan, AdcSeq)														\
	if ((AdcSeq <= ADC_SEQ__16TH) && (AdcSeq >= ADC_SEQ__13TH))									\
	{																							\
	SET_IO_BITS((AdcNo + ADC_SQR1_OFFSET), ((AdcSeq - ADC_SEQ__13TH) * 5), BITMASK_5, AdcChan);	\
	}																							\
	else if ((AdcSeq <= ADC_SEQ__12TH) && (AdcSeq >= ADC_SEQ__7TH))								\
	{																							\
	SET_IO_BITS((AdcNo + ADC_SQR2_OFFSET), ((AdcSeq - ADC_SEQ__7TH) * 5), BITMASK_5, AdcChan);	\
	}																							\
	else if ((AdcSeq <= ADC_SEQ__6TH) && (AdcSeq >= ADC_SEQ__1ST))								\
	{																							\
	SET_IO_BITS((AdcNo + ADC_SQR3_OFFSET), (AdcSeq * 5), BITMASK_5, AdcChan);					\
	}																							\
	else																						\
	{																							\
	}
	*/

/*
#define ADC_SET_SEQ(AdcNo, AdcChan, AdcSeq)														\
	if ((AdcSeq <= ADC_SEQ__6TH) && (AdcSeq >= ADC_SEQ__1ST))									\
	{																							\
	SET_IO_BITS((AdcNo + ADC_SQR3_OFFSET), (AdcSeq * 5), BITMASK_5, AdcChan);					\
	}																							\
	else if ((AdcSeq <= ADC_SEQ__12TH) && (AdcSeq >= ADC_SEQ__7TH))								\
	{																							\
	SET_IO_BITS((AdcNo + ADC_SQR2_OFFSET), ((AdcSeq - ADC_SEQ__7TH) * 5), BITMASK_5, AdcChan);	\
	}																							\
	else if ((AdcSeq <= ADC_SEQ__16TH) && (AdcSeq >= ADC_SEQ__13TH))							\
	{																							\
	SET_IO_BITS((AdcNo + ADC_SQR1_OFFSET), ((AdcSeq - ADC_SEQ__13TH) * 5), BITMASK_5, AdcChan);	\
	}																							\
	else																						\
	{																							\
	}
	*/

#define ADC_GET_DUAL_MODE_DATA(AdcNo)				((IO_MEM32((AdcNo + ADC_DR_OFFSET)) >> 16) & 0xFFFF)

#define ADC_GET_DATA(AdcNo)							IO_MEM32((AdcNo + ADC_DR_OFFSET))

#ifdef __cplusplus
}
#endif

#endif  // End of ADC_H
