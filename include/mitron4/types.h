#ifndef _MITRON4_TYPES_H_
#define _MITRON4_TYPES_H_
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
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * common data types
 */
typedef int8_t B;
typedef int16_t H;
typedef int32_t W;
typedef int64_t D;
typedef uint8_t UB;
typedef uint16_t UH;
typedef uint32_t UW;
typedef uint64_t UD;

typedef int8_t VB;
typedef int16_t VH;
typedef int32_t VW;
typedef int64_t VD;

typedef void *VP;
typedef void (*FP)();

typedef int_fast32_t INT;
typedef uint_fast32_t UINT;

typedef bool BOOL;

#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif

typedef int32_t FN;
typedef int32_t ER;
typedef int32_t ID;
typedef uint32_t ATR;
typedef uint32_t STAT;
typedef uint32_t MODE;
typedef int32_t PRI;
typedef uint32_t SIZE;

typedef int32_t TMO;
typedef uint32_t RELTIM;
typedef struct {
	D sec;
	W nsec;
} SYSTIM;

typedef uint32_t INHNO;

typedef int32_t VP_INT;

typedef int32_t ER_BOOL;
typedef int32_t ER_ID;
typedef int32_t ER_UINT;

typedef uint32_t RDVPTN;
typedef int32_t RDVNO;

#define TBIT_RDVPTN 32

/**
 * common constants
 */
#ifndef NULL
#define NULL (0)
#endif

#define E_OK (0)

/**
 * object attribute
 */
#define TA_NULL (0)

/**
 * timeout parameters
 */
#define TMO_POL (0)
#define TMO_FEVR (-1)
#define TMO_NBLK (-2)

#endif
