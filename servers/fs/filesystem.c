/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* filesystem.c - POSIX 環境マネージャのファイルシステム処理部分
 *
 *
 *
 * $Log: filesystem.c,v $
 * Revision 1.43  2000/06/01 08:46:47  naniwa
 * to implement getdents
 *
 * Revision 1.42  2000/05/25 08:04:53  naniwa
 * to check existing files
 *
 * Revision 1.41  2000/05/20 11:56:07  naniwa
 * to support chdir
 *
 * Revision 1.40  2000/05/06 03:52:23  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.39  2000/02/27 15:33:53  naniwa
 * to work as multi task OS
 *
 * Revision 1.38  2000/02/20 09:36:30  naniwa
 * fixed fs_register_inode, etc
 *
 * Revision 1.37  2000/02/04 15:17:47  naniwa
 * to call findport on demand
 *
 * Revision 1.36  2000/01/28 10:03:40  naniwa
 * minor fix around strncpy
 *
 * Revision 1.35  2000/01/26 08:22:10  naniwa
 * minor fix
 *
 * Revision 1.34  2000/01/22 11:01:53  naniwa
 * fixed to work correctly with relative path
 *
 * Revision 1.33  2000/01/15 15:29:01  naniwa
 * to support device read/write, etc
 *
 * Revision 1.32  1999/12/25 11:00:40  naniwa
 * fixed use of i_refcount
 *
 * Revision 1.31  1999/11/19 10:10:18  naniwa
 * add debug message
 *
 * Revision 1.30  1999/11/10 10:48:33  naniwa
 * to implement execve
 *
 * Revision 1.29  1999/05/30 04:04:52  naniwa
 * modified to creat file correctly
 *
 * Revision 1.28  1999/05/28 15:46:20  naniwa
 * add some comments
 *
 * Revision 1.27  1999/03/30 13:23:35  monaka
 * Minor change to debug defines. There is no effect if you make them by release mode.
 *
 * Revision 1.26  1999/03/24 03:54:42  monaka
 * printf() was renamed to printk().
 *
 * Revision 1.25  1999/03/21 00:42:48  monaka
 * Minor fix. Comment was changed.
 *
 * Revision 1.24  1998/06/11 15:29:58  night
 * alloca を使用しないよう変更。
 *
 * Revision 1.23  1998/01/06 16:40:04  night
 * デバッグ用の print 文の追加。
 *
 * Revision 1.22  1997/10/24 14:01:13  night
 * 変数の引数合わせを厳密にした。
 * 使用していない関数 foo() を削除した。
 *
 * Revision 1.21  1997/10/23 14:32:32  night
 * exec システムコール関係の処理の更新
 *
 * Revision 1.20  1997/10/11 16:25:19  night
 * ファイルの write 処理の修正。
 *
 * Revision 1.19  1997/09/09 13:50:32  night
 * POSIX のファイルへの書き込み処理(システムコールは、write) を追加。
 * これまでは、EP_NOSUP を返していた。
 *
 * Revision 1.18  1997/08/31 13:29:32  night
 * こまかい処理のデバッグを行った。
 *
 * Revision 1.17  1997/07/06 11:57:11  night
 * デバッグ文の指定を変更。
 *
 * Revision 1.16  1997/07/04 15:07:39  night
 * ・スペシャルファイル - デバイスドライバポートの対応表の関連処理の追加。
 * ・ファイルの読み込み処理の改訂。
 *
 * Revision 1.15  1997/07/03 14:24:30  night
 * mountroot/open 処理のバグを修正。
 *
 * Revision 1.14  1997/07/02 13:25:43  night
 * statfs システムコールの中身を作成
 *
 * Revision 1.13  1997/05/14 14:09:53  night
 * デバッグ文の修正。
 *
 * Revision 1.12  1997/05/06 12:47:11  night
 * ログの出力に dbg_printf に代わって printf を使用するように変更した。
 *
 * Revision 1.11  1997/04/28 15:27:16  night
 * デバッグ用の文を追加。
 *
 * Revision 1.10  1997/04/24 15:40:30  night
 * mountroot システムコールの実装を行った。
 *
 * Revision 1.9  1997/04/06 12:55:03  night
 * inode_buf、fs_buf の最後の要素を NULL に初期化する処理を
 * 追加。
 *
 * Revision 1.8  1997/03/25 13:34:53  night
 * ELF 形式の実行ファイルへの対応
 *
 * Revision 1.7  1996/11/20  12:09:47  night
 * fs_remove_file(), fs_remove_dir() の追加。
 *
 * Revision 1.6  1996/11/18  13:42:39  night
 * fs_check_inode()、fs_register_inode() を追加。
 *
 * Revision 1.5  1996/11/14  13:16:32  night
 * open および lookup 処理を追加。
 *
 * Revision 1.4  1996/11/10  11:53:51  night
 * デバッグ文の追加。
 *
 * Revision 1.3  1996/11/08  11:03:16  night
 * デバッグ文の追加。
 *
 * Revision 1.2  1996/11/07  12:44:19  night
 * ファイルシステム処理の中身を作成した。
 *
 * Revision 1.1  1996/11/05  15:13:46  night
 * 最初の登録
 *
 */

#include <fcntl.h>
#include <major.h>
#include <string.h>
#include "fs.h"
#include "devfs/devfs.h"

extern struct fsops sfs_fsops;

struct fs_entry {
    B *fsname;
    struct fsops *fsops;
};

static struct fs_entry fs_table[MAXFS + 1] = {

    {
    "null", NULL}, {
"sfs", &sfs_fsops},};


static struct fs fs_buf[MAX_MOUNT], *free_fs = NULL, *rootfs = NULL;
static struct inode inode_buf[MAX_INODE], *free_inode = NULL;
struct inode *rootfile = NULL;
static W mode_map[] = { R_OK, W_OK, R_OK | W_OK };



/* init_fs
 *
 */
W init_fs(void)
{
    W i;


    /* 各データ構造の初期化を行い
     * ルートファイルシステムをマウントする
     */
    for (i = 0; i < MAX_INODE - 1; i++) {
/*      dbg_printf ("Inode [%d] = 0x%x \n", i, &inode_buf[i]);
 */
	inode_buf[i].i_next = &(inode_buf[i + 1]);
    }
    inode_buf[MAX_INODE - 1].i_next = NULL;
    free_inode = &inode_buf[0];

    for (i = 0; i < MAX_MOUNT - 1; i++) {
	fs_buf[i].next = &fs_buf[i + 1];
    }
    fs_buf[MAX_MOUNT - 1].next = NULL;
    free_fs = &fs_buf[0];

    return (TRUE);
}

W get_device_info(UW major_minor, ID * port, UW * dd)
{
    device_info_t *p = device_find(major_minor);

    if (p) {
	*port = p->port;
	*dd = major_minor & 0x0000FFFF;
	return (EOK);
    }

    return (ENODEV);
}

/* file discriptor 0, 1, 2 の設定
 */
W open_special_dev(struct proc * procp)
{
    struct inode *ip;
    device_info_t *p;

    p = device_find(get_device_id(DEVICE_MAJOR_KEYBOARD, 0));
    if (p) {
	/* 標準入力の設定 */
	procp->proc_open_file[0].f_inode = ip = alloc_inode();
	procp->proc_open_file[0].f_offset = 0;
	procp->proc_open_file[0].f_omode = O_RDONLY;
	if (ip == NULL) {
	    return (ENOMEM);
	}
	ip->i_mode = S_IFCHR;
	ip->i_dev = p->id;
	ip->i_fs = rootfs;
	ip->i_index = -1;
	ip->i_size = 0;
	ip->i_nblock = 0;
	fs_register_inode(ip);
    }

    p = device_find(get_device_id(DEVICE_MAJOR_CONSOLE, 0));
    if (p) {
	/* 標準出力の設定 */
	procp->proc_open_file[1].f_inode = ip = alloc_inode();
	procp->proc_open_file[1].f_offset = 0;
	procp->proc_open_file[1].f_omode = O_WRONLY;
	if (ip == NULL) {
	    return (ENOMEM);
	}
	ip->i_mode = S_IFCHR;
	ip->i_dev = p->id;
	ip->i_fs = rootfs;
	ip->i_index = -2;
	ip->i_size = 0;
	ip->i_nblock = 0;
	fs_register_inode(ip);

	/* 標準エラー出力の設定 */
	procp->proc_open_file[2].f_inode = ip = alloc_inode();
	procp->proc_open_file[2].f_offset = 0;
	procp->proc_open_file[2].f_omode = O_WRONLY;
	if (ip == NULL) {
	    return (ENOMEM);
	}
	ip->i_mode = S_IFCHR;
	ip->i_dev = p->id;
	ip->i_fs = rootfs;
	ip->i_index = -3;
	ip->i_size = 0;
	ip->i_nblock = 0;
	fs_register_inode(ip);
    }

    return (EOK);
}


/*
 *
 */
/* alloc_fs -
 *
 */
struct fs *alloc_fs(void)
{
    struct fs *p;

    if (free_fs == NULL) {
	return (NULL);
    }

    p = free_fs;
    free_fs = free_fs->next;

    memset((B*)p, 0, sizeof(struct fs));
    return (p);
}

void dealloc_fs(struct fs *fsp)
{
    if (fsp == NULL) {
	return;
    }

    fsp->prev = NULL;
    fsp->next = free_fs;
    free_fs = fsp;
}

/* mount_root - root ファイルシステムのマウント
 *
 */
W mount_root(ID device, W fstype, W option)
{
    struct fsops *fsp;
    W err;

#ifdef FMDEBUG
    dbg_printf("device = 0x%x, fstype = %d, option = %d\n",
	       device, fstype, option);
#endif
    if ((fstype < 0) || (fstype > MAXFS)) {
	dbg_printf("ERROR: mount_root fstype error %d\n", fstype);
	return (EINVAL);
    }

    rootfile = alloc_inode();
    if (rootfile == NULL) {
	return (E_NOMEM);
    }

    rootfs = alloc_fs();
    if (rootfs == NULL) {
	dealloc_inode(rootfile);
	return (E_NOMEM);
    }

    fsp = fs_table[fstype].fsops;
    init_cache();
    err = fsp->mount(device, rootfs, rootfile);
    if (err) {
	return (err);
    }

    rootfile->i_fs = rootfs;
    rootfs->rootdir = rootfile;
    rootfs->device = device;
    rootfs->ops = *fsp;

    /* FS List の設定 */
    rootfs->next = rootfs;
    rootfs->prev = rootfs;

    fs_register_inode(rootfile);

    return (E_OK);
}


/* mount_fs
 *
 */
W
mount_fs(struct inode * deviceip,
	 struct inode * mountpoint, W option, char *fstype)
{
    struct fs *newfs;
    struct inode *newip;
    int fs_num;
    struct fsops *fsp;
    W device, err;

#ifdef FMDEBUG
    dbg_printf("fs: MOUNT: device = 0x%x, fstype = %s, option = %d\n",
	       deviceip->i_dev, fstype, option);
#endif
    for (fs_num = 1; fs_num <= MAXFS; ++fs_num) {
	if (!strcmp(fstype, fs_table[fs_num].fsname))
	    break;
    }
    if (fs_num > MAXFS) {
	return (EINVAL);
    }
    /* 既に mount されていないかどうかのチェック */
    device = deviceip->i_dev;
    newfs = rootfs;
    do {
	if (newfs->device == device) {
	    return (EBUSY);
	}
	newfs = newfs->next;
    } while (newfs != rootfs);

    newfs = alloc_fs();
    if (newfs == NULL) {
	return (ENOMEM);
    }

    newip = alloc_inode();
    if (newip == NULL) {
	return (E_NOMEM);
    }

    /* ファイルシステム情報の取り出し */
    fsp = fs_table[fs_num].fsops;
    err = fsp->mount(device, newfs, newip);
    if (err) {
	dealloc_fs(newfs);

	/* dealloc_inode は使えないので手動で free_inode list へ再登録 */
	newip->i_next = free_inode;
	newip->i_prev = NULL;
	free_inode = newip;

	return (err);
    }

    /* ファイルシステムのリストへ登録 */
    newfs->ops = *fsp;
    newfs->next = rootfs;
    newfs->prev = rootfs->prev;
    rootfs->prev->next = newfs;
    rootfs->prev = newfs;

    /* mount されるファイルシステムの root ディレクトリの登録 */
    newip->i_fs = newfs;
    newfs->rootdir = newip;
    newfs->device = device;

    /* mount point に coverfile を設定 */
    mountpoint->coverfile = newip;
    newfs->mountpoint = mountpoint;

    fs_register_inode(newip);

    return (EOK);
}


/* umount_fs
 *
 */
W umount_fs(UW device)
{
    struct fs *fsp;
    struct inode *ip;

    /* device から fsp を検索 */
    fsp = rootfs;
    do {
	if (fsp->device == device)
	    break;
	fsp = fsp->next;
    }
    while (fsp != rootfs);
    if (fsp == rootfs) {
	/* 見付からなかったか，root file system だった場合 */
	return (EINVAL);
    }

    if (fsp->rootdir->i_refcount > 1) {
	return (EBUSY);
    }

    ip = fsp->ilist;
    if (ip != ip->i_next) {
	/* マウントポイント以下のファイル/ディレクトリが使われている
	 * BUSY のエラーで返す
	 */
	return (EBUSY);
    }

    /* ファイルシステム情報を解放する */
    fsp->ops.umount(fsp);

    /* マウントポイントを解放する */
    fsp->mountpoint->coverfile = NULL;
    dealloc_inode(fsp->mountpoint);
    dealloc_inode(fsp->rootdir);

    /* 冗長 */
    fsp->mountpoint = NULL;
    fsp->rootdir = NULL;

    /* FS list から除外 */
    fsp->prev->next = fsp->next;
    fsp->next->prev = fsp->prev;
    dealloc_fs(fsp);

    return (EOK);
}


/* fs_open_file -
 *
 */
W
fs_open_file(B * path,
	     W oflag,
	     W mode,
	     struct access_info * acc,
	     struct inode * startip, struct inode ** newip)
{
    W error_no;

    if (oflag & O_CREAT) {
#ifdef FMDEBUG
	dbg_printf("fs_open_file: File creation mode.\n");
#endif
	error_no = fs_lookup(startip, path, O_RDONLY, mode, acc, newip);
	if (error_no == ENOENT) {
#ifdef FMDEBUG
	    dbg_printf("fs_open_file: call fs_create_file(%s)\n", path);
#endif
	    error_no = fs_create_file(startip, path, oflag, mode, acc, newip);
	    return (error_no);
	} else if (error_no == EOK) {
#ifdef FMDEBUG
	    dbg_printf("fs_open_file: File already exists.\n");
#endif
	    dealloc_inode(*newip);	/* fs_close() で行う処理はこれだけ */
	    /*      return (EEXIST); */
	    /* 後で mode と acc を確かめながら再度 open する */
	} else {
	    return (error_no);
	}
    }
#ifdef FMDEBUG
    /* パス名に従ってファイルをオープンする
     */
    dbg_printf
	("fs_open_file: startip = 0x%x, path = %s, oflag = %d, mode = %d\n",
	 startip, path, oflag, mode);
#endif

    error_no = fs_lookup(startip, path, oflag, mode, acc, newip);
    if (error_no) {
#ifdef FMDEBUG
	dbg_printf("fs: Cannot lookup -> return from fs_open_file ().\n");
#endif
	return (error_no);
    }

    if (oflag & O_TRUNC) {
      (*newip)->i_size = 0;
    }
    return (EOK);
}


/* fs_create_file - ファイルを作成する
 *
 */
W
fs_create_file(struct inode * startip,
	       char *path,
	       W oflag,
	       W mode, struct access_info * acc, struct inode ** newip)
{
    char parent_path[MAX_NAMELEN];
    struct inode *parent_ip;
    W parent_length;
    W error_no;

    for (parent_length = strlen(path); parent_length >= 0; parent_length--) {
	if (path[parent_length] == '/') {
	    strncpy(parent_path, path, MAX_NAMELEN - 1);
	    parent_path[MAX_NAMELEN - 1] = '\0';
	    parent_path[parent_length] = '\0';
	    break;
	}
    }

    if (parent_length < 0) {
	parent_ip = startip;
	parent_ip->i_refcount++;
	parent_length = 0;
    } else if (parent_length == 0) {
	parent_ip = rootfile;
	parent_ip->i_refcount++;
	parent_length = 1;
    } else {
	error_no = fs_lookup(startip, parent_path, O_WRONLY,
			  mode, acc, &parent_ip);
	if (error_no) {
	    return (error_no);
	}
	parent_length += 1;
    }

    if ((parent_ip->i_mode & S_IFMT) != S_IFDIR) {
	fs_close_file(parent_ip);
	return (ENOTDIR);
    }

    mode &= parent_ip->i_mode
	    & (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    error_no = OPS(parent_ip).create(parent_ip,
			&path[parent_length], oflag, mode, acc, newip);
    fs_close_file(parent_ip);
    if (error_no) {
	return (error_no);
    }
    return (EOK);
}



/* fs_close_file -
 *
 */
W fs_close_file(struct inode * ip)
{
    W error_no;

    /* 普通は inode を deallocate するときに書き出すのではないか? */
    if (ip->i_dirty) {
	error_no = fs_sync_file(ip);
	if (error_no) {
	    return (error_no);
	}
    }

    error_no = dealloc_inode(ip);
    if (error_no) {
	return (error_no);
    }

    return (error_no);
}



/* fs_lookup
 *
 * 機能
 *	ファイルをオープンする。
 *
 * 
 *
 */
W
fs_lookup(struct inode * startip,
	  char *path,
	  W oflag, W mode, struct access_info * acc, struct inode ** newip)
{
    struct inode *tmpip;
    struct fs *fsp;
    int len;
    char part[MAX_NAMELEN];
    W error_no;

#ifdef FMDEBUG
    dbg_printf("fs: fs_lookup(): start (path = \"%s\")\n", path);
#endif

    if (startip == NULL) {
	return (ENODEV);
    }

    tmpip = startip;
    if ((path[0] == '/') && (path[1] == '\0')) {
	startip->i_refcount++;
	*newip = startip;
	return (EOK);
    } else if (*path == '/') {
	path++;
    }

    /* パス名の最後に / があれば除去する */
    len = strlen(path) - 1;
    if ((len > 0) && (path[len] == '/')) {
      path[len] = 0;
    }

    tmpip->i_refcount++;
    while (*path != '\0') {
	int i;

	/* ディレクトリの実行許可のチェック */
	error_no = permit(tmpip, acc, X_OK);
	if (error_no) {
	    dealloc_inode(tmpip);
	    return (error_no);
	}

	for (i = 0; i < MAX_NAMELEN; i++) {
	    if ((*path == '/') || (*path == '\0')) {
		part[i] = '\0';
		error_no = OPS(tmpip).lookup(tmpip, part, oflag, mode, acc, newip);
		if (error_no) {
		    dealloc_inode(tmpip);
		    return (error_no);
		}
		/* ファイルシステムの root directory にいる場合 */
		if ((tmpip == *newip) && (!strcmp("..", part))) {
		    fsp = rootfs;
		    do {
			if ((fsp->mountpoint) != NULL &&
			    (fsp->rootdir == tmpip)) {
			    tmpip->i_refcount--;
			    dealloc_inode(tmpip);
			    tmpip = fsp->mountpoint;
			    tmpip->i_refcount++;
			    error_no =
				OPS(tmpip).lookup(tmpip, part, oflag, mode, acc,
					    newip);
			    break;
			}
			fsp = fsp->next;
		    } while (fsp != rootfs);
		}
		dealloc_inode(tmpip);

		/* パス名の次の要素へ 
		 */
		if (*path == '\0') {
		    /* ディレクトリの許可のチェック */
		    error_no = permit(*newip, acc, mode_map[oflag & 0x03]);
		    if (error_no)
			dealloc_inode(*newip);
		    return (error_no);
		}
		path++;
		tmpip = *newip;
		break;
	    }
	    part[i] = *path;
	    path++;
	}
    }

    return (ENAMETOOLONG);
}

/* fs_read_file -
 *
 * 機能
 *	引数で指定されたファイルの内容を読み込む
 *
 * 
 *	
 */
W fs_read_file(struct inode * ip, W start, B * buf, W length, W * rlength)
{
    W error_no;

    error_no = OPS(ip).read(ip, start, buf, length, rlength);
    if (error_no) {
	return (error_no);
    }

    return (EOK);
}



/* fs_write_file -
 *
 * 機能
 *	引数で指定されたファイルに buf の内容を書き込む
 *
 * 
 *	
 */
W fs_write_file(struct inode * ip, W start, B * buf, W length, W * rlength)
{
    ID device;
    W error_no;

    if (ip->i_mode & S_IFCHR) {
	/* スペシャルファイルだった */
	device = ip->i_dev;

	/* Write to the device. */
	if ((ip->i_dev & BLOCK_DEVICE_MASK) != 0) {
	    /* ブロック・デバイスだった */
	    if (ip->i_size <= start) {
		return (ENOSPC);
	    } else if (ip->i_size <= (start + length)) {
		length = ip->i_size - start;
	    }
	}
	error_no = write_device(device, buf, start, length, rlength);
	return (error_no);
    }

    error_no = OPS(ip).write(ip, start, buf, length, rlength);
    if (error_no) {
	return (error_no);
    }

    return (EOK);
}


/* fs_remove_file -
 *
 */
W
fs_remove_file(struct inode * startip, B * path, struct access_info * acc)
{
    char parent_path[MAX_NAMELEN];
    struct inode *parent_ip;
    W parent_length;
    W error_no;

    for (parent_length = strlen(path); parent_length >= 0; parent_length--) {
	if (path[parent_length] == '/') {
	    strncpy(parent_path, path, MAX_NAMELEN - 1);
	    parent_path[MAX_NAMELEN - 1] = '\0';
	    parent_path[parent_length] = '\0';
	    break;
	}
    }

    if (parent_length < 0) {
	parent_ip = startip;
	parent_ip->i_refcount++;
	parent_length = 0;
    } else if (parent_length == 0) {
	parent_ip = rootfile;
	parent_ip->i_refcount++;
	parent_length = 1;
    } else {
	error_no =
	    fs_lookup(startip, parent_path, O_RDWR, 0, acc, &parent_ip);
	if (error_no) {
	    return (error_no);
	}
	parent_length += 1;
    }

    error_no = OPS(parent_ip).unlink(parent_ip, &path[parent_length], acc);
    fs_close_file(parent_ip);
    if (error_no) {
	return (error_no);
    }
    return (EOK);
}


/* fs_remove_dir -
 *
 */
W fs_remove_dir(struct inode * startip, B * path, struct access_info * acc)
{
    char parent_path[MAX_NAMELEN];
    struct inode *parent_ip;
    W parent_length;
    W error_no;

    for (parent_length = strlen(path); parent_length >= 0; parent_length--) {
	if (path[parent_length] == '/') {
	    strncpy(parent_path, path, MAX_NAMELEN - 1);
	    parent_path[MAX_NAMELEN - 1] = '\0';
	    parent_path[parent_length] = '\0';
	    break;
	}
    }

    if (parent_length < 0) {
	parent_ip = startip;
	parent_ip->i_refcount++;
	parent_length = 0;
    } else if (parent_length == 0) {
	parent_ip = rootfile;
	parent_ip->i_refcount++;
	parent_length = 1;
    } else {
	error_no =
	    fs_lookup(startip, parent_path, O_RDWR, 0, acc, &parent_ip);
	if (error_no) {
	    return (error_no);
	}
	parent_length += 1;
    }

    error_no = OPS(parent_ip).rmdir(parent_ip, &path[parent_length], acc);
    fs_close_file(parent_ip);
    if (error_no) {
	return (error_no);
    }
    return (EOK);
}


/* fs_sync_file -
 *
 */
W fs_sync_file(struct inode * ip)
{
    W error_no;

    error_no = OPS(ip).sync(ip, 0);
    return (error_no);
}


/* fs_convert_path -
 *
 */
W fs_convert_path(struct inode * ip, B * buf, W length)
{
    return (ENOTSUP);
}



/* fs_statfs -
 *
 */
W fs_statfs(ID device, struct statfs * result)
{
    struct fs *p;

    for (p = rootfs; p != 0; p = p->next) {
	if (p->device == device) {
	    return p->ops.statfs(p, result);
	}
    }
    return (ENODEV);
}

/*
 * fs_mkdir
 */
W fs_make_dir(struct inode * startip,
	      char *path,
	      W mode, struct access_info * acc, struct inode ** newip)
{
    char parent_path[MAX_NAMELEN];
    struct inode *parent_ip;
    W parent_length;
    W error_no;

    error_no = fs_lookup(startip, path, O_RDONLY, mode, acc, newip);
    if (error_no == EOK) {
	dealloc_inode(*newip);	/* fs_close() で行う処理はこれだけ */
	return (EEXIST);
    } else if (error_no != ENOENT) {
	return (error_no);
    }

    for (parent_length = strlen(path); parent_length >= 0; parent_length--) {
	if (path[parent_length] == '/') {
	    strncpy(parent_path, path, MAX_NAMELEN - 1);
	    parent_path[MAX_NAMELEN - 1] = '\0';
	    parent_path[parent_length] = '\0';
	    break;
	}
    }

    if (parent_length < 0) {
	parent_ip = startip;
	parent_ip->i_refcount++;
	parent_length = 0;
    } else if (parent_length == 0) {
	parent_ip = rootfile;
	parent_ip->i_refcount++;
	parent_length = 1;
    } else {
	error_no = fs_lookup(startip, parent_path, O_WRONLY,
			  mode, acc, &parent_ip);
	if (error_no) {
	    return (error_no);
	}
	parent_length += 1;
    }

    if ((parent_ip->i_mode & S_IFMT) != S_IFDIR) {
	fs_close_file(parent_ip);
	return (ENOTDIR);
    }

    mode &= parent_ip->i_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    error_no = OPS(parent_ip).mkdir(parent_ip, &path[parent_length], mode, acc, newip);

    fs_close_file(parent_ip);
    if (error_no) {
	return (error_no);
    }
    return (EOK);
}

/*
 * fs_getdents
 */
W fs_getdents(struct inode * ip, ID caller, W offset,
	      VP buf, UW length, W * rsize, W * fsize)
{
    W error_no;

    error_no = OPS(ip).getdents(ip, caller, offset, buf, length, rsize, fsize);
    if (error_no)
	return (error_no);
    return (EOK);
}


/* fs_link_file -
 *
 */
W
fs_link_file(W procid, B * src, W srclen, B * dst, W dstlen,
	     struct access_info * acc)
{
    char parent_path[MAX_NAMELEN];
    struct inode *startip;
    struct inode *srcip, *parent_ip;
    W parent_length;
    W error_no;

    /* リンク元の i-node を get 無ければエラー */
    if (*src != '/') {
	error_no = proc_get_cwd(procid, &startip);
	if (error_no) {
	    return (error_no);
	}
    } else {
	startip = rootfile;
    }

    error_no = fs_lookup(startip, src, O_RDONLY, 0, acc, &srcip);
    if (error_no) {
	return (error_no);
    }

    /* リンク元がディレクトリならエラー */
    if ((srcip->i_mode & S_IFMT) == S_IFDIR) {
	fs_close_file(srcip);
	return (EISDIR);
    }

    /* リンク先の親ディレクトリの i-node を get */
    if (*dst != '/') {
	error_no = proc_get_cwd(procid, &startip);
	if (error_no) {
	    return (error_no);
	}
    } else {
	startip = rootfile;
    }

    for (parent_length = dstlen; parent_length >= 0; parent_length--) {
	if (dst[parent_length] == '/') {
	    strncpy(parent_path, dst, MAX_NAMELEN - 1);
	    parent_path[MAX_NAMELEN - 1] = '\0';
	    parent_path[parent_length] = '\0';
	    break;
	}
    }

    if (parent_length < 0) {
	parent_ip = startip;
	parent_ip->i_refcount++;
	parent_length = 0;
    } else if (parent_length == 0) {
	parent_ip = rootfile;
	parent_ip->i_refcount++;
	parent_length = 1;
    } else {
	error_no =
	    fs_lookup(startip, parent_path, O_RDWR, 0, acc, &parent_ip);
	if (error_no) {
	    return (error_no);
	}
	parent_length += 1;
    }

    /* ファイルシステムを跨ぐリンクにならないことをチェックする */
    if (srcip->i_fs != parent_ip->i_fs) {
	fs_close_file(parent_ip);
	fs_close_file(srcip);
	return (EXDEV);
    }

    /* 各ファイルシステムの link 関数を呼び出す */
    error_no = OPS(parent_ip).link(parent_ip, &dst[parent_length], srcip, acc);

    fs_close_file(parent_ip);
    fs_close_file(srcip);
    if (error_no) {
	return (error_no);
    }
    return (EOK);
}


/* --------=========== 細々とした関数群 ================--------- */

/* alloc_inode - 
 *
 */
struct inode *alloc_inode(void)
{
    struct inode *p;

    if (free_inode == NULL) {
	return (NULL);
    }

    p = free_inode;
    free_inode = free_inode->i_next;

    memset((B*)p, 0, sizeof(struct inode));
    p->i_prev = p->i_next = p;
    p->i_refcount = 1;
    return (p);
}



/* dealloc_inode -
 *
 */
W dealloc_inode(struct inode * ip)
{
    ip->i_refcount--;
    if (ip->i_refcount <= 0) {
	OPS(ip).close(ip);
	/* fs の register_list からの取り除き */
	if (ip->i_next == ip) {
	    ip->i_fs->ilist = NULL;
	} else {
	    if (ip->i_fs->ilist == ip) {
		ip->i_fs->ilist = ip->i_next;
	    }
	    ip->i_next->i_prev = ip->i_prev;
	    ip->i_prev->i_next = ip->i_next;
	}
	/* free_inode list へ登録 */
	ip->i_next = free_inode;
	ip->i_prev = NULL;
	free_inode = ip;
    }
    return (EOK);
}


/* fs_check_inode -
 *
 */
struct inode *fs_check_inode(struct fs *fsp, W index)
{
    struct inode *ip, *register_list;

    register_list = fsp->ilist;
    if (register_list == NULL) {
	return (NULL);
    }

    if (register_list->i_index == index) {
	return (register_list);
    }

    for (ip = register_list->i_next; ip != register_list; ip = ip->i_next) {
	if (ip->i_index == index) {
	    return (ip);
	}
    }
    return (NULL);
}


W fs_register_inode(struct inode * ip)
{
    struct inode *register_list;

    if (ip->i_fs->ilist == NULL) {
	ip->i_next = ip;
	ip->i_prev = ip;
	ip->i_fs->ilist = ip;
    } else {
	/* Queue の最後に追加 */
	register_list = ip->i_fs->ilist;
	ip->i_prev = register_list->i_prev;
	ip->i_next = register_list;
	register_list->i_prev->i_next = ip;
	register_list->i_prev = ip;
    }

    return (EOK);
}




/*
 * permit -
 */

W permit(struct inode * ip, struct access_info * acc, UW bits)
{
    UW mode, perm_bits;
    int shift;

    mode = ip->i_mode;
    if (acc->uid == SU_UID) {
	if (((mode & S_IFMT) == S_IFDIR) ||
	    (mode & (X_OK << 6 | X_OK << 3 | X_OK))) {
	    perm_bits = R_OK | W_OK | X_OK;
	} else {
	    perm_bits = R_OK | W_OK;
	}
    } else {
	if (acc->uid == ip->i_uid)
	    shift = 6;
	else if (acc->gid == ip->i_gid)
	    shift = 3;
	else
	    shift = 0;
	perm_bits = (mode >> shift) & 0x03;
    }

    if ((perm_bits | bits) != perm_bits)
	return (EACCES);
    return (EOK);
}
