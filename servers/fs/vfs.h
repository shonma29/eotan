/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/

/* posix_fs.h - POSIX ファイルシステム
 *
 *
 * $Log: posix_fs.h,v $
 * Revision 1.21  2000/06/23 09:18:12  naniwa
 * to support O_APPEND
 *
 * Revision 1.20  2000/06/01 08:46:49  naniwa
 * to implement getdents
 *
 * Revision 1.19  2000/05/06 03:52:25  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.18  1999/05/28 15:46:43  naniwa
 * sfs ver 1.1
 *
 * Revision 1.17  1999/03/24 04:52:08  monaka
 * Source file cleaning for avoid warnings.
 *
 * Revision 1.16  1997/10/24 13:58:39  night
 * 関数定義の追加。
 *
 * Revision 1.15  1997/10/11 16:25:19  night
 * ファイルの write 処理の修正。
 *
 * Revision 1.14  1997/09/09 13:50:32  night
 * POSIX のファイルへの書き込み処理(システムコールは、write) を追加。
 * これまでは、EP_NOSUP を返していた。
 *
 * Revision 1.13  1997/08/31 13:31:35  night
 * lseek システムコール用の定義を追加。
 *
 * Revision 1.12  1997/07/09 15:03:05  night
 * inode 構造体に i_dirty という要素を追加。
 * この要素は、構造体が変更されており、かつディスクに反映されていないこと
 * を示す。
 *
 * Revision 1.11  1997/07/04 15:07:39  night
 * ・スペシャルファイル - デバイスドライバポートの対応表の関連処理の追加。
 * ・ファイルの読み込み処理の改訂。
 *
 * Revision 1.10  1997/07/02 13:26:01  night
 * FS_MOUNTROOT マクロの引数を修正。
 *
 * Revision 1.9  1997/04/25 13:00:38  night
 * struct statfs 構造体の定義を追加。
 *
 * Revision 1.8  1997/03/25 13:34:53  night
 * ELF 形式の実行ファイルへの対応
 *
 * Revision 1.7  1996/11/20  12:10:20  night
 * FILE_UNLINK マクロ の追加。
 *
 * Revision 1.6  1996/11/18  13:43:20  night
 * inode の要素 coverfs を coverfile に変更。
 *
 * Revision 1.5  1996/11/14  13:16:54  night
 * 各種定義の追加。
 *
 * Revision 1.4  1996/11/12  11:31:42  night
 * ユーザプログラムの時には、POSIX マネージャの関数の extern 宣言を有効に
 * しないようにした。
 *
 * Revision 1.3  1996/11/08  11:04:01  night
 * O_RDONLY, FS_FMT_REG, FS_FMT_DIR, FS_FMT_DEV マクロの追加。
 *
 * Revision 1.2  1996/11/07  21:12:04  night
 * 文字コードを EUC に変更した。
 *
 * Revision 1.1  1996/11/07  12:49:19  night
 * 最初の登録
 *
 *
 */

#ifndef __FS_VFS_H__
#define __FS_VFS_H__	1

#include <fs/sfs.h>
#include <sys/statvfs.h>
#include "types.h"

#define F_FILE		0x0001
#define F_PIPE		0x0002

#define BLOCK_SIZE	512

#define BLOCK_DEVICE_MASK	(0x80000000UL)

/* file system types. */

struct fsops {
    W(*mount) ();
    W(*unmount) ();
    W(*statvfs) ();
    W(*lookup) ();
    W(*create) ();
    W(*close) ();
    W(*read) ();
    W(*write) ();
    W(*link) ();
    W(*unlink) ();
    W(*sync) ();
    W(*mkdir) ();
    W(*rmdir) ();
    W(*getdents) ();
    W(*stat) ();
    W(*wstat) ();
    W(*permit) ();
};

#define OPS(ip) ((ip)->i_fs->ops)

struct fs {
    struct fs *prev;
    struct fs *next;
    W typeid;
    struct fsops ops;
    UW device;
    struct inode *ilist;	/* 使用中の inode のリスト */
    struct inode *rootdir;
    struct inode *mountpoint;
    W dirty;
    union {
	struct sfs_superblock sfs_fs;
    } private;
};

struct inode {
    struct inode *i_prev;
    struct inode *i_next;
    struct fs *i_fs;
    UW i_device;
    UW i_lock;
    W i_refcount;
    W i_dirty;		/* この Inode は変更されており、ファイル上に変更が */
			/* 反映されていない */
    struct inode *coverfile;
    /* もし、*ここ* が マウントポイントの時には、 */
    /* この要素を実際のファイルとして処理する */

    /* in disk */
    UW i_mode;
    UW i_index;
    UW i_dev;			/* if device file */
    UW i_size;
    UW i_nblock;

    /* ここに各ファイルシステムの独自の情報が入る (union で... ) */
    union {
	struct sfs_inode sfs_inode;
    } i_private;
};

struct file
{
  struct inode		*f_inode;
  W			f_flag;
  off_t			f_offset;	/* current offset */
  W			f_omode;
};



/* filesystem.c */
extern W init_fs(void);
extern W get_device_info(UW major_minor, ID * port, UW * dd);
extern struct inode *alloc_inode(void);
extern W dealloc_inode(struct inode *);
extern struct fs *alloc_fs(void);
extern void dealloc_fs(struct fs *);

extern W fs_open_file(B * path, W oflag, W mode, struct permission *acc,
		      struct inode *startip, struct inode **newip);
extern W fs_close_file(struct inode *ip);
extern W fs_lookup(struct inode *startip, char *path, W oflag,
		   W mode, struct permission *acc, struct inode **newip);
extern W fs_create_file(struct inode *startip, char *path, W oflag,
			W mode, struct permission *acc,
			struct inode **newip);
extern W fs_sync_file(struct inode *ip);
extern W fs_read_file(struct inode *ip, W start, B * buf, W length,
		      W * rlength);
extern W fs_write_file(struct inode *ip, W start, B * buf, W length,
		       W * rlength);
extern W fs_remove_file(struct inode *startip, B * path,
			struct permission *acc);
extern W fs_remove_dir(struct inode *startip, B * path,
		       struct permission *acc);
extern W fs_statvfs(ID device, struct statvfs *result);
extern W fs_getdents(struct inode *ip, ID caller, W offset,
		     VP buf, UW length, W * rsize, W * fsize);
extern W fs_link_file(W procid, B * src, B * dst, struct permission * acc);
extern W fs_make_dir(struct inode * startip,
		     char *path, W mode, struct permission * acc,
		     struct inode ** newip);

extern W mount_root(ID device, W fstype, W option);
extern W mount_fs(struct inode *device, struct inode *mountpoint, W option,
		  char *fstype);
extern W unmount_fs(UW device);

extern struct inode *fs_check_inode(struct fs *fsp, W index);
extern W fs_register_inode(struct inode *ip);
extern W init_special_file(void);
extern W fs_convert_path(struct inode *ip, B * buf, W length);

extern struct inode *rootfile;

#endif				/* __FS_VFS_H__ */
