/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_fork.c,v 1.3 1999/11/10 10:39:32 naniwa Exp $  */

#include <errno.h>
#include <string.h>
#include <mpu/io.h>
#include <mpu/config.h>
#include <fs.h>
#include "posix.h"

/* fork 用エントリールーチン */
static int fork_entry()
{
    ena_int();
    return (0);
}

/* fork 
 *
 */
int
_fork (int esp, int ebp, int ebx, int ecx, int edx, int esi, int edi)
{
    ER error;
    struct posix_request req;
    struct posix_response *res = (struct posix_response*)&req;
    ID child_main;
    ID child_signal;
    ID myself;
    T_CTSK task_info;

    /* 子プロセスのタスク生成
     * 実行開始アドレスは、fork_entry に設定しておく。
     *
     */

    /* 子プロセスの main task の生成 */
    memset(&task_info, 0, sizeof(task_info));
    task_info.tskatr = TA_HLNG;
    task_info.startaddr = (void*)fork_entry;
    task_info.itskpri = POSIX_TASK_LEVEL;
    task_info.stksz = POSIX_STACK_SIZE;
    child_main = acre_tsk(&task_info);
    if (child_main < 0)
	return (child_main);

    /* 子プロセスの signal task の生成 */
    /* 現時点では生成しない */
    child_signal = 0;

    /* 子プロセスの UNIQ PAGE の取得 */

    /* POSIX manager の呼び出し 

     * 引数を設定して、POSIX manager にメッセージを送る。
     */
    req.param.par_fork.main_task = child_main;
    req.param.par_fork.signal_task = child_signal;

    error = _make_connection(PSC_FORK, &req);
    if (error != E_OK) {
	/* What should I do? */
	del_tsk(child_main);
	errno = error;
	return (-1);
    } else if (res->errno) {
	del_tsk(child_main);
	errno = res->errno;
	return (-1);
    }

    /* stack のコピー */
    get_tid(&myself);
    vcpy_stk(myself, esp, ebp,
	     ebx, ecx, edx, esi, edi,
	     child_main);

    /* FPU を有効にする */
    vuse_fpu(myself);

    /* 子プロセスを有効にする */
    sta_tsk(child_main, 0);
    return (res->status);
}
