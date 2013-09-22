/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
#ifndef SYS_FILE_H
#define SYS_FILE_H

struct fsops {
    W(*fs_mount) ();
    W(*fs_mountroot) ();
    W(*fs_umount) ();
    W(*fs_statfs) ();
    W(*fs_syncfs) ();
    W(*fs_read_inode) ();
    W(*fs_write_inode) ();
    W(*fs_read_super) ();
    W(*fs_write_super) ();
    W(*fs_get_inode) ();	/* open */
    W(*fs_put_inode) ();	/* close */
};

struct fs {
    struct fs *fs_prev;
    struct fs *fs_next;
    W fs_magicid;
    W fs_typeid;
    W fs_refcount;
    W fs_rflag;
    struct fsops *fs_ops;
    W fs_lock;
    UW fs_device;
    struct inode *fs_ilist;	/* 使用中の inode のリスト */
    W fs_blksize;
    struct inode *rootdir;
    struct inode *mountpoint;
    W fs_dirty;

    W fs_allblock;
    W fs_freeblock;
    W fs_usedblock;

    W fs_allinode;
    W fs_freeinode;
    W fs_usedinode;

    UW fs_isearch;		/* この番号以下の inode は使用中 */
    UW fs_bsearch;		/* この番号以下の block は使用中 */

    union {
	struct sfs_superblock sfs_fs;
    } fs_private;
};

struct inode {
    struct inode *i_prev;
    struct inode *i_next;
    struct fs *i_fs;
    UW i_device;
    UW i_lock;
    struct iops *i_ops;
    W i_refcount;
    W i_dirty;		/* この Inode は変更されており、ファイル上に変更が */
			/* 反映されていない */
    struct inode *coverfile;
    /* もし、*ここ* が マウントポイントの時には、 */
    /* この要素を実際のファイルとして処理する */

    /* in disk */
    UW i_mode;
    UW i_link;
    UW i_index;
    UW i_uid;
    UW i_gid;
    UW i_dev;			/* if device file */
    UW i_size;
    UW i_atime;
    UW i_ctime;
    UW i_mtime;
    UW i_size_blk;

    /* ここに各ファイルシステムの独自の情報が入る (union で... ) */
    union {
	struct sfs_inode sfs_inode;
    } i_private;
};

struct iops {
    W(*i_lookup) ();
    W(*i_create) ();
    W(*i_close) ();
    W(*i_read) ();
    W(*i_write) ();
    W(*i_stat) ();
    W(*i_truncate) ();
    W(*i_link) ();
    W(*i_unlink) ();
    W(*i_symlink) ();		/* not used */
    W(*i_chmod) ();
    W(*i_chown) ();
    W(*i_chgrp) ();
    W(*i_rename) ();
    W(*i_sync) ();
    W(*i_mkdir) ();
    W(*i_rmdir) ();
    W(*i_getdents) ();
};

struct file
{
  struct inode		*f_inode;
  W			f_flag;
  W			f_offset;	/* current offset */
  W			f_omode;
};

#endif
