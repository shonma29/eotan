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

/**
 * common data types
 */
typedef char B;
typedef short H;
typedef long W;
typedef long long D;
typedef unsigned char UB;
typedef unsigned short UH;
typedef unsigned long UW;
typedef unsigned long long UD;

typedef char VB;
typedef short VH;
typedef long VW;
typedef long long VD;

typedef void *VP;
typedef void (*FP)();

typedef long INT;
typedef unsigned long UINT;

typedef bool BOOL;

#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif

typedef long FN;
typedef long ER;
typedef long ID;
typedef unsigned long ATR;
typedef unsigned long STAT;
typedef unsigned long MODE;
typedef long PRI;
typedef unsigned long SIZE;

typedef long TMO;
typedef unsigned long RELTIM;
typedef struct {
	D sec;
	W nsec;
} SYSTIM;

typedef unsigned long INHNO;

typedef long VP_INT;

typedef long ER_BOOL;
typedef long ER_ID;
typedef long ER_UINT;

typedef unsigned long RDVPTN;
typedef long RDVNO;

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
