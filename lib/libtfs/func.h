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
#include "vfs.h"

extern vfs_operation_t sfs_fsops;

/* block.c */
extern W sfs_alloc_block (vfs_t *fsp);
extern W sfs_get_block_num (vfs_t *fsp,
			    struct sfs_inode *ip, W blockno);
extern W sfs_set_block_num (vfs_t *fsp,
	       struct sfs_inode *ip, W blockno, W newblock);
extern W sfs_free_block (vfs_t *fsp, W blockno);
extern W sfs_free_indirect(vfs_t *fsp, struct sfs_inode *ip,
			   int offset, int inblock);
extern W sfs_free_dindirect(vfs_t *fsp, struct sfs_inode *ip,
			    int offset, int dinblock, int inblock);
extern W sfs_free_all_dindirect(vfs_t *fsp,
				struct sfs_inode *ip, int inblock);

/* dir.c */
extern W sfs_read_dir (vnode_t *ip, W nentry, struct sfs_dir *dirp);
extern W sfs_write_dir (vnode_t *parentp, W nentry, struct sfs_dir *dirp);
extern int sfs_getdents (vnode_t *ip, ID caller, W offset, VP buf,
		       UW length, W *rsize, W *fsize);
extern int sfs_i_lookup (vnode_t *parent, char *fname, vnode_t **retip);
extern int sfs_i_link (vnode_t * parent, char *fname, vnode_t * srcip);
extern int sfs_i_unlink (vnode_t *parent, char *fname, vnode_t *ip);
extern int sfs_i_mkdir (vnode_t *parent, char *fname, W mode,
			     struct permission *acc, vnode_t **retip);
extern int sfs_i_rmdir (vnode_t *parent, char *fname, vnode_t *ip);

/* inode.c */
extern W	sfs_read_inode (vfs_t *fsp, W index, vnode_t *ip);
extern W	sfs_alloc_inode (vfs_t *fsp, vnode_t *ip);
extern W	sfs_free_inode (vfs_t *fsp, vnode_t *ip);
extern int sfs_stat(vnode_t *, struct stat *);
extern int sfs_wstat(vnode_t *);
extern int sfs_i_close (vnode_t * ip);

/* file.c */
extern int sfs_i_create (vnode_t *parent, char *fname,
			      W mode, struct permission *acc,
			      vnode_t **retip);
extern int sfs_i_read (vnode_t *ip, W start, B *buf, W length, W *rlength);
extern int sfs_i_write (vnode_t *ip, W start, B *buf, W size,
			     W *rsize);
extern W	sfs_i_truncate (vnode_t *ip, W newsize);

/* fs.c */
extern int sfs_syncfs (vfs_t *fsp);

#endif /* __SFS_FUNC_H__ */
