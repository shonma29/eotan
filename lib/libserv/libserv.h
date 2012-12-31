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
#include "../../include/itron/types.h"

/* strncpy2.c */
extern void		strncpy2(B *s1, B *s2, W size);

/* sys_debug.c */
extern W dbg_puts(B *msg);
extern W dbg_printf(B *format, ...);

/* malloc.c */
extern ER	init_malloc (UW free_memory_erea);
extern VP	malloc (UW size);
extern VP	calloc (UW size, UW nentry);
extern void	free (VP addr);

#endif /* __LIBKERN_LIBKERNEL_H__ */
