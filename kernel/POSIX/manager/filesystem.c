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

#include "posix.h"

extern int strlen(char *);
extern int strcmp(char *, char *);

struct fs_entry {
    B *fsname;
    struct fsops *fsops;
} fs_table[MAXFS + 1] = {

    {
    "null", NULL}, {
"sfs", &sfs_fsops},};


struct fs fs_buf[MAX_MOUNT], *free_fs = NULL, *rootfs = NULL;
struct inode inode_buf[MAX_INODE], *free_inode = NULL, *rootfile = NULL;
#ifdef notdef
static int use_count = 0;
#endif


/* スペシャルファイルとデバイスドライバとの対応を取るためのテーブル
 *
 * POSIX 環境では、スペシャルファイルはメジャー番号とマイナー番号で識別している。
 * B-Free OS では、デバイスドライバは識別名とポート番号、そして dd で区別しているので、
 * このような対応表が必要になる。
 */

struct special_file special_file_table[] = {
/* キャラクタデバイス */
/* major/minor        name         port      dd         handler */
    {0x00000000, "manager.wconsole", 0, 0x00000000, 0},
    {0x00010000, "driver.keyboard", 0, 0x00000000, 0},
    {0x00020000, "driver.psaux", 0, 0x00000000, 0},	/* psaux driver */

/* ブロック型デバイス */
    {0x80000000, "driver.fd", 0, 0x00000000, 0},
    {0x80010000, "driver.ide", 0, 0x00000000, 0},	/* 0 番目の IDE デバイスの全体 */
    {0x80010001, "driver.ide", 0, 0x00000001, 0},	/* 0 番目の IDE デバイスのパーティション 1 */
    {0x80010002, "driver.ide", 0, 0x00000002, 0},	/* 0 番目の IDE デバイスのパーティション 2 */
    {0x80010003, "driver.ide", 0, 0x00000003, 0},	/* 0 番目の IDE デバイスのパーティション 3 */
    {0x80010004, "driver.ide", 0, 0x00000004, 0},	/* 0 番目の IDE デバイスのパーティション 4 */
    {0x80010005, "driver.ide", 0, 0x00000005, 0},	/* 0 番目の IDE デバイスのパーティション 5 (拡張パーティション) */
    {0x80010006, "driver.ide", 0, 0x00000006, 0},	/* 0 番目の IDE デバイスのパーティション 6 (拡張パーティション) */
    {0x80010007, "driver.ide", 0, 0x00000007, 0},	/* 0 番目の IDE デバイスのパーティション 7 (拡張パーティション) */
    {0x80010008, "driver.ide", 0, 0x00000008, 0},	/* 0 番目の IDE デバイスのパーティション 8 (拡張パーティション) */
    {0x80010009, "driver.ide", 0, 0x00000009, 0},	/* 0 番目の IDE デバイスのパーティション 9 (拡張パーティション) */
    {0x80020000, "driver.ramdisk", 0, 0x00000000, 0}	/* RAMDISK */
};

UW special_file_table_entry =
    sizeof(special_file_table)/sizeof(struct special_file);

static W mode_map[] = { R_BIT, W_BIT, R_BIT | W_BIT };



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
	fs_buf[i].fs_next = &fs_buf[i + 1];
    }
    fs_buf[MAX_MOUNT - 1].fs_next = NULL;
    free_fs = &fs_buf[0];

#ifdef notdef
    /* 必要になったときに個別に内容を更新するので，初期化は不要 */
    init_special_file();
#endif
#ifdef notdef
    dbg_printf("special file table size is %d\n", special_file_table_entry);
#endif

    return (SUCCESS);
}


/* special_file_table のエントリを埋める
 */
W init_special_file()
{
    W i;
    ER error;
    ID port;

    for (i = 0; i < special_file_table_entry; i++) {
#ifdef DEBUG
	dbg_printf("find port: %s\n", special_file_table[i].name);
#endif
	error = find_port(special_file_table[i].name, &port);
	if (error) {
	    dbg_printf("Cannot access special file(%s).\n",
		       special_file_table[i].name);
	} else {
#ifdef DEBUG
	    dbg_printf("find: driver %s use port %d.\n",
		       special_file_table[i].name, port);	/* */
#endif
	    special_file_table[i].port = port;
	}
    }

    return (EP_OK);
}


W get_device_info(UW major_minor, ID * port, UW * dd)
{
    UW i;
    ER errno;
    ID p;

    for (i = 0; i < special_file_table_entry; i++) {
#ifdef notdef
	if (special_file_table[i].major_minor == major_minor)
#else
	if ((special_file_table[i].major_minor & 0xFFFF0000) ==
	    (major_minor & 0xFFFF0000))	/* major 番号のみで比較する */
#endif
	{

	    if (special_file_table[i].port <= 0) {
		errno = find_port(special_file_table[i].name, &p);
		if (errno) {
#ifdef FMDEBUG
		    dbg_printf("Cannot access special file(%s).\n",
			       special_file_table[i].name);
#endif
		    return (EP_NODEV);
		}
		special_file_table[i].port = p;
	    }

	    *port = special_file_table[i].port;
#ifdef notdef
	    *dd = special_file_table[i].dd;
#else
	    /* minor 番号が dd に対応する． */
	    /* IDE driver ではパーティションの区別に利用する */
	    *dd = major_minor & 0x0000FFFF;
#endif
	    return (EP_OK);
	}
    }
    return (EP_NODEV);
}

/* file discriptor 0, 1, 2 の設定
 */
W open_special_dev(struct proc * procp)
{
    struct inode *ip;

    /* 標準入力の設定 */
    procp->proc_open_file[0].f_inode = ip = alloc_inode();
    procp->proc_open_file[0].f_offset = 0;
    procp->proc_open_file[0].f_omode = O_RDWR;
    if (ip == NULL) {
	return (EP_NOMEM);
    }
    ip->i_mode = FS_FMT_DEV;
    ip->i_dev = special_file_table[1].major_minor;
    ip->i_fs = rootfs;
    ip->i_index = -1;
    ip->i_size = 0;
    ip->i_size_blk = 0;
    fs_register_inode(ip);

    /* 標準出力の設定 */
    procp->proc_open_file[1].f_inode = ip = alloc_inode();
    procp->proc_open_file[1].f_offset = 0;
    procp->proc_open_file[1].f_omode = O_RDWR;
    if (ip == NULL) {
	return (EP_NOMEM);
    }
    ip->i_mode = FS_FMT_DEV;
    ip->i_dev = special_file_table[0].major_minor;
    ip->i_fs = rootfs;
    ip->i_index = -2;
    ip->i_size = 0;
    ip->i_size_blk = 0;
    fs_register_inode(ip);

    /* 標準エラー出力の設定 */
    procp->proc_open_file[2].f_inode = ip = alloc_inode();
    procp->proc_open_file[2].f_offset = 0;
    procp->proc_open_file[2].f_omode = O_RDWR;
    if (ip == NULL) {
	return (EP_NOMEM);
    }
    ip->i_mode = FS_FMT_DEV;
    ip->i_dev = special_file_table[0].major_minor;
    ip->i_fs = rootfs;
    ip->i_index = -3;
    ip->i_size = 0;
    ip->i_size_blk = 0;
    fs_register_inode(ip);

    return (EP_OK);
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
    free_fs = free_fs->fs_next;

    bzero((B *) p, sizeof(struct fs));
    return (p);
}

void dealloc_fs(struct fs *fsp)
{
    if (fsp == NULL) {
	return;
    }

    fsp->fs_prev = NULL;
    fsp->fs_next = free_fs;
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
	return (EP_INVAL);
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
    err = FS_MOUNTROOT(fsp, device, rootfs, rootfile);
    if (err) {
	return (err);
    }

    rootfile->i_fs = rootfs;
    rootfs->rootdir = rootfile;
    rootfs->fs_device = device;
    rootfs->fs_ops = fsp;

    /* FS List の設定 */
    rootfs->fs_next = rootfs;
    rootfs->fs_prev = rootfs;

    fs_register_inode(rootfile);

#ifdef notdef
    {
	ID pid;
	get_tid(&pid);
	dbg_printf("rootfile = 0x%x, PID = %d\n", rootfile, pid);
	dbg_printf("ROOT FS information:\n");
	dbg_printf("typeid = %d(0x%x)\n", rootfs->fs_typeid,
		   rootfs->fs_typeid);
	dbg_printf("block size = %d\n", rootfs->fs_blksize);
	dbg_printf("all block number = %d, free block number = %d\n",
		   rootfs->fs_allblock, rootfs->fs_freeblock);
	dbg_printf("all inode number = %d, free inode number = %d\n",
		   rootfs->fs_allinode, rootfs->fs_freeinode);
    }
#endif

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
    dbg_printf("[PM] MOUNT: device = 0x%x, fstype = %s, option = %d\n",
	       deviceip->i_dev, fstype, option);
#endif
    for (fs_num = 1; fs_num <= MAXFS; ++fs_num) {
	if (!strcmp(fstype, fs_table[fs_num].fsname))
	    break;
    }
    if (fs_num > MAXFS) {
	return (EP_INVAL);
    }
    /* 既に mount されていないかどうかのチェック */
    device = deviceip->i_dev;
    newfs = rootfs;
    do {
	if (newfs->fs_device == device) {
	    return (EP_BUSY);
	}
	newfs = newfs->fs_next;
    } while (newfs != rootfs);

    newfs = alloc_fs();
    if (newfs == NULL) {
	return (EP_NOMEM);
    }

    newip = alloc_inode();
    if (newip == NULL) {
	return (E_NOMEM);
    }

    /* ファイルシステム情報の取り出し */
    fsp = fs_table[fs_num].fsops;
    err = FS_MOUNT(fsp, device, newfs, newip);
    if (err) {
	dealloc_fs(newfs);

	/* dealloc_inode は使えないので手動で free_inode list へ再登録 */
	newip->i_next = free_inode;
	newip->i_prev = NULL;
	free_inode = newip;

	return (err);
    }

    /* ファイルシステムのリストへ登録 */
    newfs->fs_ops = fsp;
    newfs->fs_next = rootfs;
    newfs->fs_prev = rootfs->fs_prev;
    rootfs->fs_prev->fs_next = newfs;
    rootfs->fs_prev = newfs;

    /* mount されるファイルシステムの root ディレクトリの登録 */
    newip->i_fs = newfs;
    newfs->rootdir = newip;
    newfs->fs_device = device;

    /* mount point に coverfile を設定 */
    mountpoint->coverfile = newip;
    newfs->mountpoint = mountpoint;

    fs_register_inode(newip);

    return (EP_OK);
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
	if (fsp->fs_device == device)
	    break;
	fsp = fsp->fs_next;
    }
    while (fsp != rootfs);
    if (fsp == rootfs) {
	/* 見付からなかったか，root file system だった場合 */
	return (EP_INVAL);
    }

    if (fsp->rootdir->i_refcount > 1) {
#ifdef notdef
      printk("[PM] dir busy: refcount %d\n", fsp->rootdir->i_refcount);
#endif
	return (EP_BUSY);
    }

    ip = fsp->fs_ilist;
    if (ip != ip->i_next) {
	/* マウントポイント以下のファイル/ディレクトリが使われている
	 * BUSY のエラーで返す
	 */
#ifdef notdef
      printk("[PM] file busy: inode %d(%d) %d(%d)\n",
	     ip->i_index, ip->i_refcount,
	     ip->i_next->i_index, ip->i_next->i_refcount);
#endif
	return (EP_BUSY);
    }

    /* ファイルシステム情報を解放する */
    FS_UMOUNT(fsp->fs_ops, fsp);

    /* マウントポイントを解放する */
    fsp->mountpoint->coverfile = NULL;
    dealloc_inode(fsp->mountpoint);
    dealloc_inode(fsp->rootdir);
#if 1
    /* 冗長 */
    fsp->mountpoint = NULL;
    fsp->rootdir = NULL;
#endif

    /* FS list から除外 */
    fsp->fs_prev->fs_next = fsp->fs_next;
    fsp->fs_next->fs_prev = fsp->fs_prev;
    dealloc_fs(fsp);

    return (EP_OK);
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
    W errno;

    if (oflag & O_CREAT) {
#ifdef FMDEBUG
	dbg_printf("fs_open_file: File creation mode.\n");
#endif
	errno = fs_lookup(startip, path, O_RDONLY, mode, acc, newip);
	if (errno == EP_NOENT) {
#ifdef FMDEBUG
	    dbg_printf("fs_open_file: call fs_create_file(%s)\n", path);
#endif
	    errno = fs_create_file(startip, path, oflag, mode, acc, newip);
	    return (errno);
	} else if (errno == EP_OK) {
#ifdef FMDEBUG
	    dbg_printf("fs_open_file: File already exists.\n");
#endif
	    dealloc_inode(*newip);	/* fs_close() で行う処理はこれだけ */
	    /*      return (EP_EXIST); */
	    /* 後で mode と acc を確かめながら再度 open する */
	} else {
	    return (errno);
	}
    }
#ifdef FMDEBUG
    /* パス名に従ってファイルをオープンする
     */
    dbg_printf
	("fs_open_file: startip = 0x%x, path = %s, oflag = %d, mode = %d\n",
	 startip, path, oflag, mode);
#endif

    errno = fs_lookup(startip, path, oflag, mode, acc, newip);
    if (errno) {
#ifdef FMDEBUG
	dbg_printf("[PM] Cannot lookup -> return from fs_open_file ().\n");
#endif
	return (errno);
    }

    if (oflag & O_TRUNC) {
      (*newip)->i_size = 0;
    }
    return (EP_OK);
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
#ifdef USE_ALLOCA
    char *parent_path;
#else
    char parent_path[MAX_NAMELEN];
#endif
    struct inode *parent_ip;
    W parent_length;
    W errno;

    for (parent_length = strlen(path); parent_length >= 0; parent_length--) {
	if (path[parent_length] == '/') {
#ifdef USE_ALLOCA
	    parent_path = alloca(parent_length + 1);
	    if (parent_path == NULL) {
		return (EP_NOMEM);
	    }
	    /* kernlib の strncpy の仕様にあわせるため */
	    strncpy(parent_path, path, parent_length + 1);
#else
	    strncpy(parent_path, path, MAX_NAMELEN);
#endif
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
	errno = fs_lookup(startip, parent_path, O_WRONLY,
			  mode, acc, &parent_ip);
	if (errno) {
	    return (errno);
	}
	parent_length += 1;
    }

    if ((parent_ip->i_mode & FS_FMT_MSK) != FS_FMT_DIR) {
	fs_close_file(parent_ip);
	return (EP_NOTDIR);
    }

    errno = FILE_CREATE(parent_ip,
			&path[parent_length], oflag, mode, acc, newip);
    fs_close_file(parent_ip);
    if (errno) {
	return (errno);
    }
    return (EP_OK);
}



/* fs_close_file -
 *
 */
W fs_close_file(struct inode * ip)
{
    W errno;

    /* 普通は inode を deallocate するときに書き出すのではないか? */
    if (ip->i_dirty) {
	errno = fs_sync_file(ip);
	if (errno) {
	    return (errno);
	}
    }

    errno = dealloc_inode(ip);
    if (errno) {
	return (errno);
    }

    return (errno);
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
#ifdef USE_ALLOCA
    char *part;
#else
    char part[MAX_NAMELEN];
#endif
    W errno;

#ifdef FMDEBUG
    dbg_printf("[PM] fs_lookup(): start (path = \"%s\")\n", path);
#endif

#ifdef USE_ALLOCA
    part = alloca(MAX_NAMELEN);
    if (part == NULL) {
	return (EP_NOMEM);
    }
#endif

    if (startip == NULL) {
#ifdef notdef
	dbg_printf("fs_lookup: startip address is NULL\n");
#endif
	return (EP_NODEV);
    }

    tmpip = startip;
    if ((path[0] == '/') && (path[1] == '\0')) {
	startip->i_refcount++;
	*newip = startip;
	return (EP_OK);
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
	errno = permit(tmpip, acc, X_BIT);
	if (errno) {
	    dealloc_inode(tmpip);
	    return (errno);
	}

	for (i = 0; i < MAX_NAMELEN; i++) {
	    if ((*path == '/') || (*path == '\0')) {
		part[i] = '\0';
#ifdef notdef
		dbg_printf("[PM] lookup of part: \"%s\"\n", part);
		dbg_printf("file_lookup():1 %d\n", __LINE__);
#endif
		errno = FILE_LOOKUP(tmpip, part, oflag, mode, acc, newip);
		if (errno) {
#ifdef notdef
		    dbg_printf("[PM] fs_lookup: not entry.\n");
#endif
		    dealloc_inode(tmpip);
		    return (errno);
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
			    errno =
				FILE_LOOKUP(tmpip, part, oflag, mode, acc,
					    newip);
			    break;
			}
			fsp = fsp->fs_next;
		    } while (fsp != rootfs);
		}
		dealloc_inode(tmpip);

		/* パス名の次の要素へ 
		 */
		if (*path == '\0') {
		    /* ディレクトリの許可のチェック */
		    errno = permit(*newip, acc, mode_map[oflag & 0x03]);
		    if (errno)
			dealloc_inode(*newip);
		    return (errno);
		}
		path++;
		tmpip = *newip;
		break;
	    }
	    part[i] = *path;
	    path++;
	}
    }

    return (EP_NAMETOOLONG);
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
    W errno;

    errno = FILE_READ(ip, start, buf, length, rlength);
    if (errno) {
	return (errno);
    }

    return (EP_OK);
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
    W errno;
    extern W sfs_write_device(ID, B *, W, W, W *);

    if (ip->i_mode & FS_FMT_DEV) {
	/* スペシャルファイルだった */
	device = ip->i_dev;

	/* Write to the device. */
	if ((ip->i_dev & BLOCK_DEVICE_MASK) != 0) {
	    /* ブロック・デバイスだった */
	    if (ip->i_size <= start) {
		return (EP_NOSPC);
	    } else if (ip->i_size <= (start + length)) {
		length = ip->i_size - start;
	    }
	}
	errno = sfs_write_device(device, buf, start, length, rlength);
	return (errno);
    }

    errno = FILE_WRITE(ip, start, buf, length, rlength);
    if (errno) {
	return (errno);
    }

    return (EP_OK);
}


/* fs_remove_file -
 *
 */
W
fs_remove_file(struct inode * startip, B * path, struct access_info * acc)
{
#ifdef USE_ALLCOA
    char *parent_path;
#else
    char parent_path[MAX_NAMELEN];
#endif
    struct inode *parent_ip;
    W parent_length;
    W errno;

    for (parent_length = strlen(path); parent_length >= 0; parent_length--) {
	if (path[parent_length] == '/') {
#ifdef USE_ALLOCA
	    parent_path = alloca(parent_length + 1);
	    if (parent_path == NULL) {
		return (EP_NOMEM);
	    }
	    /* kernlib の strncpy の仕様にあわせるため */
	    strncpy(parent_path, path, parent_length + 1);
#else
	    strncpy(parent_path, path, MAX_NAMELEN);
#endif
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
	errno =
	    fs_lookup(startip, parent_path, O_RDWR, 0, acc, &parent_ip);
	if (errno) {
	    return (errno);
	}
	parent_length += 1;
    }

    errno = FILE_UNLINK(parent_ip, &path[parent_length], acc);
    fs_close_file(parent_ip);
    if (errno) {
	return (errno);
    }
    return (EP_OK);
}


/* fs_remove_dir -
 *
 */
W fs_remove_dir(struct inode * startip, B * path, struct access_info * acc)
{
#ifdef USE_ALLOCA
    char *parent_path;
#else
    char parent_path[MAX_NAMELEN];
#endif
    struct inode *parent_ip;
    W parent_length;
    W errno;

    for (parent_length = strlen(path); parent_length >= 0; parent_length--) {
	if (path[parent_length] == '/') {
#ifdef USE_ALLOCA
	    parent_path = alloca(parent_length + 1);
	    if (parent_path == NULL) {
		return (EP_NOMEM);
	    }
	    /* kernlib の strncpy の仕様にあわせるため */
	    strncpy(parent_path, path, parent_length + 1);
#else
	    strncpy(parent_path, path, MAX_NAMELEN);
#endif
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
	errno =
	    fs_lookup(startip, parent_path, O_RDWR, 0, acc, &parent_ip);
	if (errno) {
	    return (errno);
	}
	parent_length += 1;
    }

    errno = DIR_UNLINK(parent_ip, &path[parent_length], acc);
    fs_close_file(parent_ip);
    if (errno) {
	return (errno);
    }
    return (EP_OK);
}


/* fs_sync_file -
 *
 */
W fs_sync_file(struct inode * ip)
{
    W errno;

    errno = FILE_SYNC(ip, 0);
    return (errno);
}


/* fs_convert_path -
 *
 */
W fs_convert_path(struct inode * ip, B * buf, W length)
{
    return (EP_NOSUP);
}



/* fs_statfs -
 *
 */
W fs_statfs(ID device, struct statfs * result)
{
    struct fs *p;

#ifdef notdef
    dbg_printf("statfs: device = %d\n", device);
#endif
    for (p = rootfs; p != 0; p = p->fs_next) {
	if (p->fs_device == device) {
	    result->f_type = p->fs_typeid;
	    result->f_bsize = p->fs_blksize;
	    result->f_blocks = p->fs_freeblock;
	    result->f_bfree = p->fs_freeblock;
	    result->f_bavail = p->fs_allblock;
	    result->f_files = p->fs_allinode;
	    result->f_free = p->fs_freeinode;
	    return (EP_OK);
	}
    }
    return (EP_NODEV);
}

/*
 * fs_mkdir
 */
W fs_make_dir(struct inode * startip,
	      char *path,
	      W mode, struct access_info * acc, struct inode ** newip)
{
#ifdef USE_ALLOCA
    char *parent_path;
#else
    char parent_path[MAX_NAMELEN];
#endif
    struct inode *parent_ip;
    W parent_length;
    W errno;

    errno = fs_lookup(startip, path, O_RDONLY, mode, acc, newip);
    if (errno == EP_OK) {
	dealloc_inode(*newip);	/* fs_close() で行う処理はこれだけ */
	return (EP_EXIST);
    } else if (errno != EP_NOENT) {
	return (errno);
    }

    for (parent_length = strlen(path); parent_length >= 0; parent_length--) {
	if (path[parent_length] == '/') {
#ifdef USE_ALLOCA
	    parent_path = alloca(parent_length + 1);
	    if (parent_path == NULL) {
		return (EP_NOMEM);
	    }
	    /* kernlib の strncpy の仕様にあわせるため */
	    strncpy(parent_path, path, parent_length + 1);
#else
	    strncpy(parent_path, path, MAX_NAMELEN);
#endif
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
	errno = fs_lookup(startip, parent_path, O_WRONLY,
			  mode, acc, &parent_ip);
	if (errno) {
	    return (errno);
	}
	parent_length += 1;
    }

    if ((parent_ip->i_mode & FS_FMT_MSK) != FS_FMT_DIR) {
	fs_close_file(parent_ip);
	return (EP_NOTDIR);
    }

    errno = DIR_CREATE(parent_ip, &path[parent_length], mode, acc, newip);

    fs_close_file(parent_ip);
    if (errno) {
	return (errno);
    }
    return (EP_OK);
}

/*
 * fs_getdents
 */
W fs_getdents(struct inode * ip, ID caller, W offset,
	      VP buf, UW length, W * rsize, W * fsize)
{
    W errno;

    errno = GET_DENTS(ip, caller, offset, buf, length, rsize, fsize);
    if (errno)
	return (errno);
    return (EP_OK);
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
    W errno;

    /* リンク元の i-node を get 無ければエラー */
    if (*src != '/') {
	errno = proc_get_cwd(procid, &startip);
	if (errno) {
	    return (errno);
	}
    } else {
	startip = rootfile;
    }

    errno = fs_lookup(startip, src, O_RDONLY, 0, acc, &srcip);
    if (errno) {
	return (errno);
    }

    /* リンク元がディレクトリならエラー */
    if ((srcip->i_mode & FS_FMT_MSK) == FS_FMT_DIR) {
	fs_close_file(srcip);
	return (EP_ISDIR);
    }

    /* リンク先の親ディレクトリの i-node を get */
    if (*dst != '/') {
	errno = proc_get_cwd(procid, &startip);
	if (errno) {
	    return (errno);
	}
    } else {
	startip = rootfile;
    }

    for (parent_length = dstlen; parent_length >= 0; parent_length--) {
	if (dst[parent_length] == '/') {
	    strncpy(parent_path, dst, MAX_NAMELEN);
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
	errno =
	    fs_lookup(startip, parent_path, O_RDWR, 0, acc, &parent_ip);
	if (errno) {
	    return (errno);
	}
	parent_length += 1;
    }

    /* ファイルシステムを跨ぐリンクにならないことをチェックする */
    if (srcip->i_fs != parent_ip->i_fs) {
	fs_close_file(parent_ip);
	fs_close_file(srcip);
	return (EP_XDEV);
    }

    /* 各ファイルシステムの link 関数を呼び出す */
    errno = FILE_LINK(parent_ip, &dst[parent_length], srcip, acc);

    fs_close_file(parent_ip);
    fs_close_file(srcip);
    if (errno) {
	return (errno);
    }
    return (EP_OK);
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

    bzero((B *) p, sizeof(struct inode));
    p->i_prev = p->i_next = p;
    p->i_refcount = 1;
#ifdef notdef
    dbg_printf("[PM] alloc_inode count = %d\n", ++use_count);
#endif
    return (p);
}



/* dealloc_inode -
 *
 */
W dealloc_inode(struct inode * ip)
{
    ip->i_refcount--;
    if (ip->i_refcount <= 0) {
	FILE_CLOSE(ip);
	/* fs の register_list からの取り除き */
	if (ip->i_next == ip) {
#if 0
	    dbg_printf("[PM] register_list become empty!!\n");
#endif
	    ip->i_fs->fs_ilist = NULL;
	} else {
	    if (ip->i_fs->fs_ilist == ip) {
		ip->i_fs->fs_ilist = ip->i_next;
	    }
	    ip->i_next->i_prev = ip->i_prev;
	    ip->i_prev->i_next = ip->i_next;
	}
	/* free_inode list へ登録 */
	ip->i_next = free_inode;
	ip->i_prev = NULL;
	free_inode = ip;
#ifdef notdef
	dbg_printf("[PM] dealloc_inode count = %d\n", --use_count);
	dbg_printf("[PM] dealloc_inode index = %d\n", ip->i_index);
#endif
    }
    return (EP_OK);
}


/* fs_check_inode -
 *
 */
struct inode *fs_check_inode(struct fs *fsp, W index)
{
    struct inode *ip, *register_list;

    register_list = fsp->fs_ilist;
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
#ifdef notdef
    dbg_printf("[PM] register_inode index = %d\n", ip->i_index);
#endif
    if (ip->i_fs->fs_ilist == NULL) {
	ip->i_next = ip;
	ip->i_prev = ip;
	ip->i_fs->fs_ilist = ip;
    } else {
	/* Queue の最後に追加 */
	register_list = ip->i_fs->fs_ilist;
	ip->i_prev = register_list->i_prev;
	ip->i_next = register_list;
	register_list->i_prev->i_next = ip;
	register_list->i_prev = ip;
    }

    return (EP_OK);
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
	if (((mode & FS_FMT_MSK) == FS_FMT_DIR) ||
	    (mode & (X_BIT << 6 | X_BIT << 3 | X_BIT))) {
	    perm_bits = R_BIT | W_BIT | X_BIT;
	} else {
	    perm_bits = R_BIT | W_BIT;
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
#ifdef notdef
    dbg_printf("[PM] uid = %d, mode = %x, permit %x\n",
	       acc->uid, mode, perm_bits);
#endif
    if ((perm_bits | bits) != perm_bits)
	return (EP_ACCESS);
    return (EP_OK);
}

/* do_df()
 */

W do_df()
{
    struct fs *fsp;
    int rate;

    fsp = rootfs;
    do {
        rate = 1024 / fsp->fs_blksize;
	printk("device:%x kbytes = %d use = %d avail = %d Mounted on %d\n",
	       fsp->fs_device,
	       fsp->fs_allblock / rate,
	       fsp->fs_usedblock / rate,
	       fsp->fs_freeblock / rate,
	       (fsp->mountpoint == NULL ? 1 : fsp->mountpoint->i_index));
	fsp = fsp->fs_next;
    } while (fsp != rootfs);
    return (EP_OK);
}
