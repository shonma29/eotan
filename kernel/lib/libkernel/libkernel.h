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
#include "../../../include/itron/types.h"
#include "../../../include/stdarg.h"

/* binary.c */
extern void	bzero (B *bufp, W size);
extern void	bcopy (UB *buf1, UB *buf2, W size);

/* string.c */
extern void		strncpy2(B *s1, B *s2, W size);

/* vnprintf.c */
extern W vnprintf(void (*out)(const B), B *format, va_list ap);

/* sys_debug.c */
extern W dbg_printf(B *format, ...);

#include "../../../include/itron/syscall.h"

/* malloc.c */
extern ER	init_malloc (UW free_memory_erea);
extern VP	malloc (UW size);
extern VP	calloc (UW size, UW nentry);
extern void	free (VP addr);

extern ER	sys_slp ();

#endif /* __LIBKERN_LIBKERNEL_H__ */
