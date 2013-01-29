/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2003, Tomohide Naniwa

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/lowlib.h,v 1.6 2000/05/25 08:03:12 naniwa Exp $ */

#ifndef __POSIX_LOWLIB_H__
#define __POSIX_LOWLIB_H__	1

#include <itron/types.h>
#include <lowlib.h>
#include "../../servers/fs/fs.h"
#include "../../servers/fs/vfs.h"
#include "../../servers/fs/mm.h"
#include "../../servers/fs/api.h"
#include "../libc/errno.h"
#include "syscall.h"
#include "global.h"
#include "funcs.h"


#define PROC_INFO	(LOWLIB_DATA)
#define MY_PID	(PROC_INFO->my_pid)
#define ERRNO	(PROC_INFO->errno)

#endif				/* __POSIX_LOWLIB_H__ */
