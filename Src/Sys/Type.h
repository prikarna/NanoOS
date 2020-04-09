#ifndef TYPE_H
#define TYPE_H

typedef unsigned int		UINT32_T;
typedef int					INT32_T;
typedef unsigned int *		UINT32_PTR_T;
typedef int *				INT32_PTR_T;
typedef unsigned short		UINT16_T;
typedef short				INT16_T;
typedef unsigned short *	UINT16_PTR_T;
typedef short *				INT16_PTR_T;
typedef unsigned char		UINT8_T;
typedef char				INT8_T;
typedef unsigned char *		UINT8_PTR_T;
typedef char *				INT8_PTR_T;

#define BOOL				UINT8_T
#define WORD				UINT32_T
#define PWORD				UINT32_PTR_T
#define HALF_WORD			UINT16_T
#define PHALF_WORD			UINT16_PTR_T
#define BYTE				UINT8_T
#define PBYTE				UINT8_PTR_T
#define CHAR				INT8_T
#define UCHAR				UINT8_T
#define PCHAR				INT8_PTR_T
#define PUCHAR				UINT8_PTR_T

#define FALSE				0
#define TRUE				1

#define CLEAR				0
#define SET					1

#define DISABLE				0
#define ENABLE				1

#define LOW					0
#define HIGH				1

#ifdef __GNUC__
# define va_list		__builtin_va_list
# define va_start(v,l)	__builtin_va_start(v,l)
# define va_end(v)		__builtin_va_end(v)
# define va_arg(v,l)	__builtin_va_arg(v,l)
#else
# error va_xxx must be defined for other compiler or include related header which define them, e.g., stdarg.h
#endif  // End of __GNUC__

#ifndef _IN
# define _IN
# define _OUT
# define _IN_OPT
# define _OUT_OPT
#endif

#ifndef NULL
# define NULL		0
#endif

#endif  // End of TYPE_H
