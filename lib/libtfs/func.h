/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/


#ifndef __SFS_FUNC_H__
#define __SFS_FUNC_H__	1

#include <core/types.h>
#include <fs/tfs.h>
#include <fs/vfs.h>
#include <sys/stat.h>

extern vfs_operation_t vfs_fsops;

/* inode.c */
extern W	sfs_read_inode (vfs_t *fsp, W index, vnode_t *ip);
extern int sfs_stat(vnode_t *, struct stat *);
extern int sfs_i_close (vnode_t * ip);

/* file.c */
extern int sfs_i_create(vnode_t *parent, const char *fname,
			      W mode, struct permission *acc,
			      vnode_t **retip);
extern W	sfs_i_truncate (vnode_t *ip, W newsize);

#include "funcs.h"

#endif /* __SFS_FUNC_H__ */
