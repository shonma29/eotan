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

#include "../include/string.h"
#include "../include/itron/types.h"
#include "../include/itron/errno.h"
#include "thread.h"
#include "func.h"
#include "lowlib.h"
#include "mpu/mpufunc.h"



struct lowlib_info lowlib_table[MAX_MODULE];
W nlowlib = 0;


/*
 *	引数で指定した lowlib モジュールの初期化
 */
ER init_lowlib(struct module_info *modp)
{
    ER(*init) (struct lowlib_info *);
    ER errno;

    if (modp->type != lowlib) {
	return (E_PAR);
    }


    printk("LOWLIB: call init function in module.\n");

    init = modp->entry;
    errno = (init) (&lowlib_table[nlowlib]);
    if (errno == E_OK) {
	memcpy(lowlib_table[nlowlib].name, modp->name, MAX_MODULE_NAME);
	lowlib_table[nlowlib].modp = modp;
	printk("LOWLIB: [%d] install %s module\n", nlowlib, modp->name);
	nlowlib++;
    } else {
	printk("loading error (errno = %d)\n", errno);
    }

    return (errno);
}



/*
 *	指定したタスクに LOWLIB をくっつける
 */
ER load_lowlib(VP * argp)
{
    struct a {
	ID task;
	B *name;
    } *args = (struct a *) argp;

    W i;
    T_TCB *tskp;
#ifdef MONAKA
    VP ppage;
    struct lowlib_data *plowlib;
#endif
    ER errno;
    struct lowlib_data ld;
    UW *paddr;

    if (args->task < 0) {
	return (E_ID);
    }
    if (args->name == NULL) {
	return (E_PAR);
    }

    tskp = get_thread_ptr(args->task);
    if (tskp == NULL) {
	return (E_ID);
    }


    for (i = 0; i < nlowlib; i++) {
	if (strncmp(args->name, lowlib_table[i].name, MAX_MODULE_NAME) ==
	    0) {
#ifdef notdef
	    printk("Found module %s == %s\n", args->name,
		   lowlib_table[i].name);
#endif
	    /* 該当するモジュールを発見した。 */

	    /* タスクに割り込み処理関数を登録する */
	    if (lowlib_table[i].intr_func) {
		if (tskp->n_interrupt >= MAX_MODULE) {
		    return (E_SYS);
		}

		tskp->interrupt[tskp->n_interrupt].intr_no =
		    lowlib_table[i].intr;
		tskp->interrupt[tskp->n_interrupt].intr_func =
		    lowlib_table[i].intr_func;
		tskp->n_interrupt++;
	    }
#ifdef notdef
	    printk("Registed interrupt functions to the task.\n");
#endif

	    /* lowlib が使用するタスク毎に異なる情報(タスク ID など) の
	     * 領域を設定する。
	     * 使用する仮想メモリ領域 (LOWLIB_DATA) を物理メモリにマップする。
	     * あ、その前に Region の設定もする必要があるかも。。
	     */

	    if (tskp->n_interrupt == 1) {
		paddr = (UW *) vtor(args->task, (UW) LOWLIB_DATA);
		if (paddr == NULL) {
		    errno = region_map(args->task, LOWLIB_DATA,
				     sizeof(struct lowlib_data), ACC_USER);
		    if (errno) {
			return (errno);
		    }
		} else {
		    printk
			("WARNING: LOWLIB_DATA has already been mapped\n");
		}
		memset(&ld, 0, sizeof(struct lowlib_data));
		errno = region_put(args->task, LOWLIB_DATA,
				 sizeof(struct lowlib_data), &ld);
		if (errno)
		    return (errno);
	    }
#ifdef MONAKA
	    plowlib = (struct lowlib_data *) ppage;
	    memset(plowlib, 0, sizeof(struct lowlib_data));
#endif
	    return (E_OK);
	}
    }


    return (E_PAR);
}

/*
 *	LOWLIB の情報を返す
 */
ER stat_lowlib(VP * argp)
{
    struct a {
	B *name;
	W *nlowlib;		/* lowlib の登録数 (name == */
	/* NULL のとき)             */
	struct lowlib_info *infop;
    } *args = (struct a *) argp;
    W i;

    if ((args->name == NULL) && (args->infop == NULL)) {
	printk("sts_low: nlowlib = %d\n", nlowlib);
	*(args->nlowlib) = nlowlib;
	return (E_OK);
    }

    if (args->infop == NULL) {
	return (E_PAR);
    }

    if (args->name == NULL) {
	if ((*(args->nlowlib) < 0) || (*(args->nlowlib) >= nlowlib)) {
	    return (E_PAR);
	}

	memcpy(args->infop, &lowlib_table[*(args->nlowlib)],
	      sizeof(struct lowlib_info));
	return (E_OK);
    }

    for (i = 0; i < nlowlib; i++) {
	if (strncmp(args->name, lowlib_table[i].name, MAX_MODULE_NAME) ==
	    0) {
	    /* 該当するモジュールを発見した。 */

	    memcpy(args->infop, &lowlib_table[i],
		  sizeof(struct lowlib_info));
	    return (E_OK);
	}
    }

    return (E_ID);
}
