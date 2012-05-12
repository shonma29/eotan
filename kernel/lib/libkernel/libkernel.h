/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2003, Tomohide Naniwa

*/
/* @(#) $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/libkernel.h,v 1.1 1999/04/18 17:48:33 monaka Exp $ */

#ifndef __LIBKERN_LIBKERNEL_H__
#define __LIBKERN_LIBKERNEL_H__


/* binary.c */
extern void	bzero (B *bufp, W size);
extern void	bcopy (UB *buf1, UB *buf2, W size);


/* message_port.c */
extern ID	get_port (W minsize, W maxsize);
extern W	get_req (ID port, VP request, W *size);



/* port_manager.c */
extern PORT_MANAGER_ERROR	find_port (B *name, ID *rport);
extern ID			alloc_port (W size, W max_entry);


/* string.c */
extern void		strncpy (B *s1, B *s2, W size);


/* sys_debug.c */
extern W	dbg_printf (B *fmt, ...);


#include <itron_sysc.h>


/* malloc.c */
extern ER	init_malloc (UW free_memory_erea);
extern VP	malloc (UW size);
extern VP	calloc (UW size, UW nentry);
extern void	free (VP addr);




extern ER	sys_slp ();


extern PORT_MANAGER_ERROR	regist_port (PORT_NAME *name, ID port);



#endif /* __LIBKERN_LIBKERNEL_H__ */
