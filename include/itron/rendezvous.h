#ifndef _ITRON_RENDEZVOUH_H_
#define _ITRON_RENDEZVOUH_H_

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
#include "types.h"

typedef int ER_UINT;

typedef unsigned int RDVPTN;
typedef int RDVNO;

#define TBIT_RDVPTN 32

typedef struct t_cpor {
	ATR poratr;
	UINT maxcmsz;
	UINT maxrmsz;
} T_CPOR;

typedef struct t_rpor {
	ID ctskid;
	ID atskid;
} T_RPOR;

typedef struct t_rrdv {
	ID wtskid;
} T_RRDV;


extern ER cre_por(ID porid, T_CPOR *pk_cpor);
extern ER_ID acre_por(T_CPOR *pk_cpor);
extern ER del_por(ID porid);
extern ER_UINT cal_por(ID porid, RDVPTN calptn, VP msg, UINT cmsgsz);
extern ER_UINT acp_por(ID porid, RDVPTN acpptn, RDVNO *p_rdvno, VP msg);
extern ER rpl_rdv(RDVNO rdvno, VP msg, UINT rmsgsz);

#endif