/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/


#ifndef __SFS_FUNC_H__
#define __SFS_FUNC_H__	1

/* common definitions */
#define SFS_BLOCK_SIZE 512


/* sfs_block.c */
extern W sfs_alloc_block (W fd, struct fs *fsp);
extern W sfs_get_block_num (W fd, struct fs *fsp,
			    struct sfs_inode *ip, W blockno);
extern W sfs_get_indirect_block_num (W fd, struct fs *fsp,
				     struct sfs_inode *ip, W blockno);
extern W sfs_get_dindirect_block_num (W fd, struct fs *fsp,
				      struct sfs_inode *ip, W blockno);
extern W sfs_set_block_num (W fd, struct fs *fsp,
	       struct sfs_inode *ip, W blockno, W newblock);
extern W sfs_set_indirect_block_num (W fd, struct fs *fsp, 
				     struct sfs_inode *ip, W blockno, W newblock);
extern W sfs_set_dindirect_block_num (W fd, struct fs *fsp,
				      struct sfs_inode *ip, W blockno, W newblock);
extern W sfs_free_block (W fd, struct fs *fsp, W blockno);
extern W sfs_free_indirect(W fd, struct fs *fsp, struct sfs_inode *ip,
			   int offset, int inblock);
extern W sfs_free_dindirect(W fd, struct fs *fsp, struct sfs_inode *ip,
			    int offset, int dinblock, int inblock);
extern W sfs_free_all_dindirect(W fd, struct fs *fsp,
				struct sfs_inode *ip, int inblock);

/* sfs_dir.c */
extern W sfs_read_dir (struct inode *ip, W nentry, struct sfs_dir *dirp);
extern W sfs_write_dir (struct inode *parentp, W nentry, struct sfs_dir *dirp);
extern W sfs_getdents (struct inode *ip, ID caller, W offset, VP buf,
		       UW length, W *rsize, W *fsize);

/* sfs_inode.c */
extern W	sfs_get_inode_offset (struct fs *fsp, W ino);
extern W	sfs_read_inode (struct fs *fsp, W index, struct inode *ip);
extern W	sfs_alloc_inode (ID fd, struct fs *fsp);
extern W	sfs_write_inode (W fd, struct fs *fsp, struct sfs_inode *ip);
extern W	sfs_free_inode (struct fs *fsp, struct inode *ip);


/* sfs_file.c */
/* extern W	sfs_i_open (); */
extern W	sfs_i_lookup (struct inode *parent, char *fname, W oflag,
			      W mode, struct access_info *acc,
			      struct inode **retip);
extern W	sfs_i_create (struct inode *parent, char *fname, W oflag,
			      W mode, struct access_info *acc,
			      struct inode **retip);
extern W	sfs_i_close ();
extern W	sfs_i_read (struct inode *ip, W start, B *buf, W length, W *rlength);
extern W	sfs_i_write (struct inode *ip, W start, B *buf, W size,
			     W *rsize);
extern W	sfs_i_stat ();
extern W	sfs_i_truncate (struct inode *ip, W newsize);
extern W	sfs_i_link ();
extern W	sfs_i_unlink (struct inode *parent, char *fname,
			      struct access_info *acc);
extern W	sfs_i_symlink ();
extern W	sfs_i_chmod ();
extern W	sfs_i_chown ();
extern W	sfs_i_chgrp ();
extern W	sfs_i_rename ();
extern W	sfs_i_sync (struct inode *ip);
extern W	sfs_i_mkdir (struct inode *parent, char *fname, W mode,
			     struct access_info *acc, struct inode **retip);
extern W	sfs_i_rmdir (struct inode *parent, char *fname,
			     struct access_info *acc);

/* sfs_fs.c */
extern W sfs_mountroot (ID device, struct fs *rootfsp, struct inode *rootfile);
extern W sfs_mount (ID device, struct fs *rootfsp, struct inode *rootfile);
extern W sfs_syncfs (struct fs *fsp, W umflag);
extern W sfs_umount(struct fs * rootfsp);
extern W sfs_statfs();
extern W sfs_get_inode();
extern W sfs_put_inode();

/* sfs_bitmap.c */
extern void sfs_init_bitmap_cache(void);
extern W sfs_alloc_bitmap(W fd, W blockno, B **ptr, int **dirty);
extern W sfs_sync_bitmap(W fd);

#endif /* __SFS_FUNC_H__ */

