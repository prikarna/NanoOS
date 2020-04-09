/*
 * File    : Config.h
 * Remark  : Definition of NanoOS configuration
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "Sys\Type.h"

#pragma pack(push)
#pragma pack(1)

#define CONFIG_STR_LEN			16
#define DEF_APP_NAME			"NApplication"

#define CFG__ENABLE_AUTO_RUN	FALSE
#define CFG__CONTROL			1

struct _CONFIG {
	UINT8_T		AutoRun;
	UINT8_T		Control;
	INT8_T		AppName[CONFIG_STR_LEN];
	UINT32_T	AppSize;
	UINT32_T	SRAMUsage;
};

typedef struct _CONFIG		CONFIG, *PCONFIG;

#pragma pack(pop)

#endif  // End of CONFIG_H
