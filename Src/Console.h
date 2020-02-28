/*
 * File    : Console.h
 * Remark  : Definition of built-in console of NanoOS
 *
 */

#ifndef CONSOLE_H
#define CONSOLE_H

#include "Type.h"

#define CONS_VER		0x0101
#define CONS_MJ_VER		((CONS_VER >> 8) & 0xFF)
#define CONS_MN_VER		(CONS_VER & 0xFF)

/*
 * Console interface
 */
#define CONS__INPUT_BUFFER_LENGTH		128

#define CONS_CMD__VER			0
#define CONS_CMD__HELP			1
#define CONS_CMD__EXIT			2
#define CONS_CMD__LED			3
#define CONS_CMD__RUN			4
#define CONS_CMD__THREAD		5
#define CONS_CMD__TERM			6
#define CONS_CMD__SUSPEND		7
#define CONS_CMD__RESUME		8
#define CONS_CMD__STRESS		9
#define CONS_CMD__DUMP			10
#define CONS_CMD__RESET			11
#define CONS_CMD__CONFIG		12
#define CONS_CMD__AUTO_RUN		13
#define CONS_CMD__APP_PRIV		14
#define CONS_CMD__DEBUG			15
#define CONS_CMD__LOCK_STRESS	16
#define CONS_CMD__USB_SEND		17
#define CONS_CMD__USB_RECV		18

#endif  // End of CONSOLE_H
