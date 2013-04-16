#ifndef _BTRON_PROCESS_H_
#define _BTRON_PROCESS_H_

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

/* terminate option */
#define TERM_NRM 0x0000
#define TERM_ALL 0x0001

/* priority option */
#define P_ABS 0x0000
#define P_REL 0x0000
#define P_TASK 0x0000

/* priority range */
#define MIN_PROC_PRI 0
#define MAX_PROC_PRI 255

#define MIN_ABS_PRI 0
#define MAX_ABS_PRI 127
#define MIN_TSS1_PRI 128
#define MAX_TSS1_PRI 191
#define MIN_TSS2_PRI 192
#define MAX_TSS2_PRI 255

/* status */
#define P_WAIT 0x2000
#define P_READY 0x4000
#define P_RUN 0x8000

typedef struct {
	UW state;
	W priority;
	W parpid;
} P_STATE;

/* user */
typedef struct {
	TC usr_name[14];
	TC grp_name1[14];
	TC grp_name2[14];
	TC grp_name3[14];
	TC grp_name4[14];
	W level;
	W net_level;
} P_USER;

/* message */
typedef struct {
	W msg_type;
	W msg_size;
	UB msg_body[0];
} MESSAGE;

/* statistics */
typedef struct {
	UW etime;
	UW utime;
	UW stime;
	W tmem;
	W wmem;
	W resv[11];
} P_INFO;


extern WERR cre_prc(LINK *lnk, W pri, MESSAGE *msg);
extern VOID ext_prc(W code);
extern ERR ter_prc(W pid, W code, W opt);
extern WERR chg_pri(W id, W pri, W opt);
extern WERR prc_sts(W pid, P_STATE *buff, TC *name);
extern ERR chg_usr(P_USER *buff);
extern WERR get_usr(W pid, P_USER *buff);
extern ERR get_inf(W pid, P_INFO *buff);
extern WERR cre_tsk(FP entry, W pri, W arg);
extern VOID ext_tsk();
extern ERR ter_tsk(W tskid);
extern ERR slp_tsk(W time);
extern ERR wup_tsk(W tskid);
extern WERR can_wup(W tskid);
extern ERR dly_tsk(W time);
extern W get_tid();

#endif
