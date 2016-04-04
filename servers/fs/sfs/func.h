/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/


#ifndef __SFS_FUNC_H__
#define __SFS_FUNC_H__	1

#include <sys/stat.h>

extern struct fsops	sfs_fsops;

/* block.c */
extern W sfs_alloc_block (W fd, struct fs *fsp);
extern W sfs_get_block_num (W fd, struct fs *fsp,
			    struct sfs_inode *ip, W blockno);
extern W sfs_set_block_num (W fd, struct fs *fsp,
	       struct sfs_inode *ip, W blockno, W newblock);
extern W sfs_free_block (W fd, struct fs *fsp, W blockno);
extern W sfs_free_indirect(W fd, struct fs *fsp, struct sfs_inode *ip,
			   int offset, int inblock);
extern W sfs_free_dindirect(W fd, struct fs *fsp, struct sfs_inode *ip,
			    int offset, int dinblock, int inblock);
extern W sfs_free_all_dindirect(W fd, struct fs *fsp,
				struct sfs_inode *ip, int inblock);

/* dir.c */
extern W sfs_read_dir (struct inode *ip, W nentry, struct sfs_dir *dirp);
extern W sfs_write_dir (struct inode *parentp, W nentry, struct sfs_dir *dirp);
extern W sfs_getdents (struct inode *ip, ID caller, W offset, VP buf,
		       UW length, W *rsize, W *fsize);

/* inode.c */
extern W	sfs_read_inode (struct fs *fsp, W index, struct inode *ip);
extern W	sfs_alloc_inode (ID fd, struct fs *fsp);
extern W	sfs_write_inode (W fd, struct fs *fsp, struct sfs_inode *ip);
extern W	sfs_free_inode (struct fs *fsp, struct inode *ip);
extern W sfs_stat(struct inode *, struct stat *);
extern W sfs_wstat(struct inode *);
extern W sfs_permit(struct inode *ip, struct permission *acc, UW bits);

/* file.c */
extern W	sfs_i_lookup (struct inode *parent, char *fname, W oflag,
			      W mode, struct permission *acc,
			      struct inode **retip);
extern W	sfs_i_create (struct inode *parent, char *fname, W oflag,
			      W mode, struct permission *acc,
			      struct inode **retip);
extern W	sfs_i_close ();
extern W	sfs_i_read (struct inode *ip, W start, B *buf, W length, W *rlength);
extern W	sfs_i_write (struct inode *ip, W start, B *buf, W size,
			     W *rsize);
extern W	sfs_i_truncate (struct inode *ip, W newsize);
extern W	sfs_i_link ();
extern W	sfs_i_unlink (struct inode *parent, char *fname,
			      struct permission *acc);
extern W	sfs_i_sync (struct inode *ip);
extern W	sfs_i_mkdir (struct inode *parent, char *fname, W mode,
			     struct permission *acc, struct inode **retip);
extern W	sfs_i_rmdir (struct inode *parent, char *fname,
			     struct permission *acc);

/* fs.c */
extern W sfs_mount (ID device, struct fs *rootfsp, struct inode *rootfile);
extern W sfs_syncfs (struct fs *fsp, W umflag);
extern W sfs_unmount(struct fs * rootfsp);

#endif /* __SFS_FUNC_H__ */
