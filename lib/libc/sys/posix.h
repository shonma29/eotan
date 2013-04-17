/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native.h,v 1.2 2000/06/01 08:43:10 naniwa Exp $ */


#ifndef __POSIX_H__

#include <unistd.h>
#include <local.h>
#include <itron/errno.h>
#include <itron/types.h>
#include <itron/syscall.h>
#include <sys/syscall.h>

extern thread_local_t *local_data;

extern ER _make_connection(W wOperation, struct posix_request *req);
extern W _call_fs(W wOperation, struct posix_request *req);

#endif
