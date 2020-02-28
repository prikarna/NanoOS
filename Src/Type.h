/*
 * File    : Type.h
 * Remark  : Some type definitions, used by NanoOS.
 *
 */

#ifndef TYPE_H
#define TYPE_H

typedef char				INT8_T;
typedef char *				INT8_PTR_T;
typedef unsigned char		UINT8_T;
typedef unsigned char *		UINT8_PTR_T;
typedef short				INT16_T;
typedef short *				INT16_PTR_T;
typedef unsigned short		UINT16_T;
typedef unsigned short *	UINT16_PTR_T;
typedef int					INT32_T;
typedef int *				INT32_PTR_T;
typedef unsigned int		UINT32_T;
typedef unsigned int *		UINT32_PTR_T;

#ifndef BOOL
# define BOOL		UINT8_T
# define PBOOL		UINT8_PTR_T
#endif

#ifndef FALSE
# define FALSE		0
# define TRUE		1
#endif

#ifndef DISABLE
# define DISABLE	0
# define ENABLE		1
#endif

#ifndef LOW
# define LOW		0
# define HIGH		1
#endif

#ifdef __GNUC__
# define va_list			__builtin_va_list
# define va_start(v,l)		__builtin_va_start(v,l)
# define va_end(v)			__builtin_va_end(v)
# define va_arg(v,l)		__builtin_va_arg(v,l)
#else
# error va_xxx must be defined for other compiler
#endif

#ifndef _IN
# define _IN
# define _OUT
# define _IN_OPT
# define _OUT_OPT
#endif

#ifndef NULL
# define NULL		0
#endif

#endif   // End of TYPE_H
