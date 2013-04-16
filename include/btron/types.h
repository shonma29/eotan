#ifndef _BTRON_TYPES_H_
#define _BTRON_TYPES_H_

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

/**
 * basic data types
 */
typedef char B;
typedef short H;
typedef int W;
typedef long long D;
typedef unsigned char UB;
typedef unsigned short UH;
typedef unsigned int UW;
typedef unsigned long long UD;

typedef char VB;
typedef short VH;
typedef int VW;
typedef long long VD;

typedef void *VP;
typedef void (*FP)();

/* forbid optimization */
typedef volatile char _B;
typedef volatile short _H;
typedef volatile int _W;
typedef volatile long long _D;

typedef volatile unsigned char _UB;
typedef volatile unsigned short _UH;
typedef volatile unsigned int _UW;
typedef volatile unsigned long long _UD;
typedef volatile void _void;

/**
 * modification
 */
#define CONST const
#define LOCAL static
#define EXPORT
#define IMPORT extern
#define VOID void

/**
 * numbers
 */
typedef float FLOAT;
typedef double DOUBLE;
typedef int BOOL;
typedef W (*FUNCP)();

/**
 * boolean
 */
typedef enum {
	False = 0,
	True = 1
} Bool;

/**
 * id
 */
typedef W ID;

/**
 * time
 */
typedef W MSEC;
/*
typedef W STIME;
*/
typedef W NSEC;
typedef D STIME;

/**
 * character code
 */
/*typedef UH TC;*/
typedef UW TC;
#define TNULL (TC)0)

/**
 * error code
 */
#ifdef BIG_ENDIAN
typedef union {
	W err;
	struct {
		H eclass;
		UH detail;
	} c;
} ErrCode;
#else
typedef union {
	W err;
	struct {
		UH detail;
		H eclass;
	} c;
} ErrCode;
#endif

/* return value of system call */
typedef W ERR;
typedef W WERR;

#define OK (0)

/**
 * common definition
 */
#define CLR 0x0000
#define NOCLR 0x0008
#define DELEXIT 0x8000

/**
 * size
 */
typedef struct Size {
	H h;
	H v;
} SIZE;

/**
 * point
 */
typedef struct point {
	H x;
	H y;
} PNT;

/**
 * rectangle
 */
typedef union rect {
	struct _rect {
		H left;
		H top;
		H right;
		H bottom;
	} c;
	struct {
		PNT lefttop;
		PNT rightbot;
	} p;
} RECT;

/**
 * link to file
 */
#define L_FSNM (20)
typedef struct {
	TC fs_name[L_FSNM];
	UH f_id;
	UH atr1;
	UH atr2;
	UH atr3;
	UH atr4;
	UH atr5;
} LINK;

/**
 * window
 */
typedef W WID;

#endif
