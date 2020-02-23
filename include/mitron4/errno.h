#ifndef _MITRON4_ERRNO_H_
#define _MITRON4_ERRNO_H_
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
#include <core/types.h>

/**
 * inner errors
 */
#define E_SYS (-5)

/**
 * support errors
 */
#define E_NOSPT (-9)
#define E_RSFN (-10)
#define E_RSATR (-11)

/**
 * parameter errors
 */
#define E_PAR (-17)
#define E_ID (-18)

/**
 * context errors
 */
#define E_CTX (-25)
#define E_MACV (-26)
#define E_OACV (-27)
#define E_ILUSE (-28)

/**
 * resource errors
 */
#define E_NOMEM (-33)
#define E_NOID (-34)

/**
 * object status errors
 */
#define E_OBJ (-41)
#define E_NOEXS (-42)
#define E_QOVR (-43)

/**
 * release errors
 */
#define E_RLWAI (-49)
#define E_TMOUT (-50)
#define E_DLT (-51)
#define E_CLS (-52)

/**
 * warnings
 */
#define E_WBLK (-57)
#define E_BOVR (-58)

/**
 * reserved errors
 */

/**
 * implement specific errors
 */


/**
 * macros
 */
static inline ER ERCD(ER mercd, ER sercd)
{
	return (sercd << 8) | mercd;
}

static inline ER MERCD(ER ercd)
{
	return ercd & 0xff;
}

static inline ER SERCD(ER ercd)
{
	return ercd >> 8;
}

#endif
