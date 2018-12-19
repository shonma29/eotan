/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/


#ifndef __SFS_FUNC_H__
#define __SFS_FUNC_H__	1

#include <fs/sfs.h>
#include <fs/vfs.h>
#include <sys/stat.h>

extern vfs_operation_t sfs_fsops;

/* dir.c */
extern int sfs_i_unlink (vnode_t *parent, char *fname, vnode_t *ip);
extern int sfs_i_rmdir (vnode_t *parent, char *fname, vnode_t *ip);

/* inode.c */
extern W	sfs_read_inode (vfs_t *fsp, W index, vnode_t *ip);
extern W	sfs_alloc_inode (vfs_t *fsp, vnode_t *ip);
extern W	sfs_free_inode (vfs_t *fsp, vnode_t *ip);
extern int sfs_stat(vnode_t *, struct stat *);
extern int sfs_wstat(vnode_t *);
extern int sfs_i_close (vnode_t * ip);

/* file.c */
extern int sfs_i_create(vnode_t *parent, const char *fname,
			      W mode, struct permission *acc,
			      vnode_t **retip);
//TODO use off_t
extern int sfs_i_write (vnode_t *ip, B *buf, W start, W size,
			     W *rsize);
extern W	sfs_i_truncate (vnode_t *ip, W newsize);

/* fs.c */
extern int sfs_syncfs (vfs_t *fsp);

#include "funcs.h"

#endif /* __SFS_FUNC_H__ */
