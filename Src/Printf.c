/*
 * File    : Printf.c
 * Remark  : Formatted print helper routines with very limited format.
 *           Format prefix is % (percent) and currently supported format are:
 *           d   = to display an integer variable in number format e.g., 2345
 *           D   = same as d
 *           c   = to display a byte variable in single character. Note
 *                 that character being displayed depends on terminal 
 *                 type, for example ASCI terminal will display character
 *                 in ASCI.
 *           s   = to display a string (C string or null terminated 
 *                 string) variable. Note that character being displayed
 *                 depends on teriminal type.
 *           x   = to display an integer variable in haxadecimal format with 
 *                 lower case letter and optional number of digit (1 to 8 digit) 
 *                 before x format, for example %2x and a variable represent by x 
 *                 is 15 then will display 0f
 *           X   = same as x but with capital letter e.g., FE34FF
 *
 */

#include "NanoOS.h"

/*
 * ASCII
 * 65 - 90  = A - Z
 * 97 - 122 = a - z
 * 48 - 57  = 0 - 9
 */

static const char	sHexCharsU[] = { "0123456789ABCDEF" };
static const char	sHexCharsL[] = { "0123456789abcdef" };

UINT32_T PrnWriteString(PRINT_CHAR_TYPE PrintChar, const char *szString)
{
	UINT32_T		i = 0;
	UINT8_PTR_T		p = (UINT8_PTR_T) szString;

	if ((!PrintChar) || (!p)) return 0;

	while (*p != '\0') {
		(* PrintChar)(*p);
		i++;
		p++;
	}

	return i;
}

UINT32_T PrnWriteNumber(PRINT_CHAR_TYPE PrintChar, UINT32_T uNumber)
{
	//
	// Assume unsinged int of uNumber has 32 bits long
	//

	unsigned int	uDiv = 0, uMod = 0, uRes = 0;
	char			buf[128];
	char *			pc;
	int				iRes = 0;

	if (!PrintChar) return 0;

	buf[0] = 0;
	pc = &buf[1];
	uRes = uNumber;

	do {
		uDiv = uRes / 10;
		uMod = uRes - (uDiv * 10);
		*pc++ = sHexCharsU[uMod];
		uRes = uDiv;
	} while (uDiv >= 10);
	if (uDiv != 0)
		*pc = sHexCharsU[uDiv];
	else
		pc--;

	while (*pc != '\0') {
		(* PrintChar)(*pc);
		pc--;
		iRes++;
	}

	return (UINT32_T) iRes;
}

UINT32_T PrnWriteHexa(PRINT_CHAR_TYPE PrintChar, UINT8_T uIsHexCapital, UINT32_T uHex, UINT32_T uDigit)
{
	//
	// Assume unsigned int of uiHex has 32 bits long = 8 nibbles = 8 digit
	//

	char			buf[8];
	char			*pc;
	unsigned int	i, uMax;
	int				ch;

	if (!PrintChar) return 0;

	uMax = (uDigit < sizeof(buf)) ? uDigit : sizeof(buf);

	pc = &buf[0];
	for (i = 0; i < uMax; i++) {
		ch = uHex & 0xF;
		if (uIsHexCapital) {
			*pc++ = sHexCharsU[ch];
		} else {
			*pc++ = sHexCharsL[ch];
		}
		uHex >>= 4;
	}

	pc--;

	for (i = 0; i < uMax; i++) {
		(* PrintChar)(*pc);
		pc--;
	}

	return i;
}

UINT32_T PrnVPrintf(PRINT_CHAR_TYPE PrintChar, const char *szFormat, va_list argList)
{
	char *			pc = (char *) szFormat;
	UINT32_T		u;
	UINT8_T			c;
	char *			s;
	UINT32_T		uRes = 0;
	UINT32_T		uCount = 0;
	UINT32_T		uDigit;

	if (pc == 0) return 0;
	if (PrintChar == 0) return 0;

	while (*pc != '\0')
	{
		c = *pc++;
		if (c != '%')
		{
			(* PrintChar)(c);
			uCount++;
		}
		else
		{
			switch ((int) *pc)
			{
			case 'd':
			case 'D':
				u = va_arg(argList, UINT32_T);
				uRes = PrnWriteNumber(PrintChar, u);
				uCount += uRes;
				break;

			case 'c':
			case 'C':
				c = (UINT8_T) va_arg(argList, UINT32_T);
				(* PrintChar)(c);
				uCount++;
				break;

			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				uDigit = (UINT32_T) *pc - 48;
				pc++;
				u = (UINT32_T) va_arg(argList, UINT32_T);
				if (*pc == 'x') {
					uRes = PrnWriteHexa(PrintChar, FALSE, u, uDigit);
					uCount += uRes;
				} else if (*pc == 'X') {
					uRes = PrnWriteHexa(PrintChar, TRUE, u, uDigit);
					uCount += uRes;
				} else {
					uRes = PrnWriteString(PrintChar, "[Invalid digit number of ");
					uCount += uRes;

					(* PrintChar)(*pc);
					uCount++;
					
					(* PrintChar)(']');
					uCount++;
				}
				break;

			case 'x':
				u = (UINT32_T) va_arg(argList, UINT32_T);
				uRes = PrnWriteHexa(PrintChar, FALSE, u, 8);
				uCount += uRes;
				break;

			case 'X':
				u = (UINT32_T) va_arg(argList, UINT32_T);
				uRes = PrnWriteHexa(PrintChar, TRUE, u, 8);
				uCount += uRes;
				break;

			case 's':
				s = va_arg(argList, INT8_PTR_T);
				uRes = PrnWriteString(PrintChar, s);
				uCount += uRes;
				break;

			case '%':
				c = (UINT8_T) va_arg(argList, UINT32_T);
				(* PrintChar)(c);
				uCount++;
				break;


			default:
				uRes = PrnWriteString(PrintChar, "[Unknown format: ");
				uCount += uRes;

				(* PrintChar)(*pc);
				uCount++;
				
				(* PrintChar)(']');
				uCount++;
				break;
			}

			pc++;
		}
	}

	return uCount;
}

/*
UINT32_T PrnPrintf(PRINT_CHAR_TYPE PrintChar, const char *szFormat, ...)
{
	UINT32_T	uRet = 0;
	va_list		argList;

	va_start(argList, szFormat);
	uRet = UtlVPrintf(PrintChar, szFormat, argList);
	va_end(argList);

	return uRet;
}
*/