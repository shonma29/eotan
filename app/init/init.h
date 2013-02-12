/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
(C) 2000-2003, Tomohide Naniwa

*/

#ifndef __INIT_H__
#define __INIT_H__	1

#include <itron/types.h>
#include <itron/errno.h>
#include "../../lib/libc/others/stdlib.h"

#define MAJOR_VERSION	1
#define MINOR_VERSION	1

extern void init_device(void);
extern W posix_init (ID myself, W root_device);
extern ER lowlib_load (ID myself, B *name);

#endif /* __INIT_H__ */
