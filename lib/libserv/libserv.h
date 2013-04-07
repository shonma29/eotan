#ifndef __LIBKERN_LIBKERNEL_H__
#define __LIBKERN_LIBKERNEL_H__
/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2003, Tomohide Naniwa

*/
/* @(#) $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/libkernel.h,v 1.1 1999/04/18 17:48:33 monaka Exp $ */
#include <itron/types.h>
#include <itron/struct.h>

/* vcre_tsk.c */
ER vcre_tsk (T_CTSK *pk_ctsk, ID *rid);

/* vnew_reg.c */
ER vnew_reg (ID id, VP start, W min, W max, UW perm, FP handle, ID *rid);

/* sys_debug.c */
extern W dbg_puts(B *msg);
extern W dbg_printf(B *format, ...);

/* malloc.c */
extern ER	init_malloc (UW free_memory_erea);
extern VP	malloc (UW size);
extern VP	calloc (UW size, UW nentry);
extern void	free (VP addr);
extern void print_freelist(void);

#endif /* __LIBKERN_LIBKERNEL_H__ */
