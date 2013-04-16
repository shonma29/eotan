#ifndef _BTRON_IPC_H_
#define _BTRON_IPC_H_

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

#include <btron/types.h>

/* time out */
typedef W TMOUT;

#define T_NOWAIT (0)
#define T_FOREVER (-1)

/* semaphore */
#define SEM_SYNC 0x0000
#define SEM_EXCL 0x4000

/* event flag */
#define WF_AND 0x0000
#define WF_OR 0x0002


extern WERR cre_sem(W cnt, UW opt);
extern ERR del_sem(ID id);
extern ERR sig_sem(ID id);
extern ERR wai_sem(ID id, TMOUT tmo);

extern WERR cre_flg(W ptn, UW opt);
extern ERR del_flg(ID id);
extern ERR set_flg(ID id, W ptn);
extern ERR clr_flg(ID id, W ptn);
extern WERR wai_flg(ID id, W ptn, UW mode, TMOUT tmo);

extern WERR cre_mbf(W bufsz, W max, UW opt);
extern ERR del_mbf(ID id);
extern ERR snd_mbf(ID id, VP msg, W sz, TMOUT tmo);
extern WERR rcv_mbf(ID id, VP msg, TMOUT tmo);

extern WERR cre_por(W maxcmsz, W maxrmsz, UW opt);
extern ERR del_por(ID id);
extern WERR cal_por(ID id, UW calptn, VP msg, W cmsz, TMOUT tmo);
extern WERR acp_por(ID id, UW acpptn, W *rdv, VP msg, TMOUT tmo);
extern ERR fwd_por(ID id, UW calptn, W rdv, VP msg, W cmsz);
extern ERR rpl_rdv(W rdv, VP msg, W rmsz);

#endif
