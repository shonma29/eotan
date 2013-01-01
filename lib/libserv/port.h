#ifndef __LIBKERN_PORT_H__
#define __LIBKERN_PORT_H__
/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2003, Tomohide Naniwa

*/
/* @(#) $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/libkernel.h,v 1.1 1999/04/18 17:48:33 monaka Exp $ */
#include <itron/types.h>
#include "../../servers/port-manager/port-manager.h"

/* port_manager.c */
extern PORT_MANAGER_ERROR	find_port (B *name, ID *rport);
extern ID			alloc_port (W size, W max_entry);

extern PORT_MANAGER_ERROR	regist_port (port_name *name, ID port);

#endif /* __LIBKERN_PORT_H__ */
