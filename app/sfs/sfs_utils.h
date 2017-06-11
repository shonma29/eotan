/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa
*/

#ifndef __APP_SFS_UTILS_H__
#define __APP_SFS_UTILS_H__	1


#include "../../include/core/types.h"
#include "../../include/fs/sfs.h"
#include "../../include/fs/vfs.h"


#define ROUNDUP(x,align)	(((((int)x) + ((align) - 1))/(align))*(align))
#define MIN(x,y)		((x > y) ? y : x)


#endif /* __MKFS_H__ */
