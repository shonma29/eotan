/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa
*/

#ifndef __MKFS_H__
#define __MKFS_H__	1


#ifndef EOTA
#include <fcntl.h>
#endif
#include "../../kernel/ITRON/h/types.h"
#include "../../servers/fs/sfs/sfs_fs.h"


#define ROUNDUP(x,align)	(((((int)x) + ((align) - 1))/(align))*(align))


#endif /* __MKFS_H__ */
