/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* lowlib.c --- lowlib 関連の関数
 *
 *
 */


/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/common/lowlib.c,v 1.8 2000/02/06 09:10:52 naniwa Exp $ */

#include <string.h>
#include <itron/types.h>
#include <itron/errno.h>
#include "thread.h"
#include "func.h"
#include "lowlib.h"
#include "mpu/mpufunc.h"


/*
 *	指定したタスクに LOWLIB をくっつける
 */
ER load_lowlib(VP * argp)
{
    struct a {
	ID task;
    } *args = (struct a *) argp;

    T_TCB *tskp;
    ER errno;
    struct lowlib_data ld;
    UW *paddr;

    if (args->task < 0) {
	return (E_ID);
    }

    tskp = get_thread_ptr(args->task);
    if (tskp == NULL) {
	return (E_ID);
    }

    paddr = (UW *) vtor(args->task, (UW) LOWLIB_DATA);
    if (paddr == NULL) {
	errno = region_map(args->task, LOWLIB_DATA,
		sizeof(struct lowlib_data), ACC_USER);
	if (errno) {
	    return (errno);
	}
    } else {
	printk("WARNING: LOWLIB_DATA has already been mapped\n");
    }

    memset(&ld, 0, sizeof(struct lowlib_data));
    errno = region_put(args->task, LOWLIB_DATA,
	    sizeof(struct lowlib_data), &ld);

    if (errno)
	return (errno);

    return (E_OK);
}
