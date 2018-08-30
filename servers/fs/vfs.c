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
#include <fs/nconfig.h>
#include <fs/vfs.h>
#include <nerve/kcall.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include "api.h"
#include "devfs/devfs.h"
#include "procfs/process.h"
#include "../../lib/libserv/libserv.h"

extern vfs_operation_t sfs_fsops;
extern vfs_operation_t devfs_fsops;

struct fs_entry {
    B *fsname;
    vfs_operation_t *fsops;
};

static struct fs_entry fs_table[] = {
    {"null", NULL},
    {"sfs", &sfs_fsops},
};


static vfs_t fs_buf[MAX_MOUNT], *rootfs = NULL;
static list_t free_fs;
static vfs_t *devfs = NULL;
vnode_t *rootfile = NULL;

static vfs_t *alloc_fs(void);
static void dealloc_fs(vfs_t *);
static W copy_path(char * parent_path, char * path, vnode_t * startip,
		vnode_t ** parent_ip);



/* init_fs
 *
 */
W fs_init(void)
{
    W i;

    /* 各データ構造の初期化を行い
     * ルートファイルシステムをマウントする
     */
    vnodes_initialize(kcall->palloc, kcall->pfree, MAX_VNODE);

    rootfile = vnodes_create();
    if (rootfile == NULL) {
	return (E_NOMEM);
    }

    list_initialize(&free_fs);
    for (i = 0; i < sizeof(fs_buf) / sizeof(fs_buf[0]); i++)
	list_append(&free_fs, &(fs_buf[i].bros));

    if (cache_initialize())
	return (E_NOMEM);

    devfs = alloc_fs();
    if (!devfs) {
	return (E_NOMEM);
    }

    devfs->operations = devfs_fsops;

    return (TRUE);
}

/* file discriptor 0, 1, 2 の設定
 */
W open_special_devices(struct proc * procp)
{
    vnode_t *ip;
    device_info_t *p;

    p = device_find(get_device_id(DEVICE_MAJOR_CONS, 0));
    if (p) {
	/* 標準入力の設定 */
	procp->session.files[0].f_inode = ip = vnodes_create();
	procp->session.files[0].f_offset = 0;
	procp->session.files[0].f_omode = O_RDONLY;
	if (ip == NULL) {
	    return (ENOMEM);
	}
	ip->mode = S_IFCHR;
	ip->dev = p->id;
	ip->fs = devfs;
	ip->index = -1;
	ip->size = p->size;
	ip->nblock = 0;
	ip->refer_count = 1;
	vnodes_append(ip);

	/* 標準出力の設定 */
	procp->session.files[1].f_inode = ip = vnodes_create();
	procp->session.files[1].f_offset = 0;
	procp->session.files[1].f_omode = O_WRONLY;
	if (ip == NULL) {
	    return (ENOMEM);
	}
	ip->mode = S_IFCHR;
	ip->dev = p->id;
	ip->fs = devfs;
	ip->index = -2;
	ip->size = p->size;
	ip->nblock = 0;
	ip->refer_count = 1;
	vnodes_append(ip);

	/* 標準エラー出力の設定 */
	procp->session.files[2].f_inode = ip = vnodes_create();
	procp->session.files[2].f_offset = 0;
	procp->session.files[2].f_omode = O_WRONLY;
	if (ip == NULL) {
	    return (ENOMEM);
	}
	ip->mode = S_IFCHR;
	ip->dev = p->id;
	ip->fs = devfs;
	ip->index = -3;
	ip->size = p->size;
	ip->nblock = 0;
	ip->refer_count = 1;
	vnodes_append(ip);
    }

    return (EOK);
}


/*
 *
 */
/* alloc_fs -
 *
 */
static vfs_t *alloc_fs(void)
{
    list_t *p = list_pick(&free_fs);
    vfs_t *fsp;

    if (p == NULL) {
	return (NULL);
    }

    fsp = getFsParent(p);

    memset((B*)fsp, 0, sizeof(vfs_t));
    list_initialize(&(fsp->bros));
    list_initialize(&(fsp->vnodes));
    return (fsp);
}

static void dealloc_fs(vfs_t *fsp)
{
    if (fsp == NULL) {
	return;
    }

    list_remove(&(fsp->bros));
    list_append(&free_fs, &(fsp->bros));
}

W find_fs(UB *fsname, W *fstype)
{
    W fs_num;

    for (fs_num = 1; fs_num < sizeof(fs_table) / sizeof(struct fs_entry); ++fs_num) {
	if (!strcmp((const char*)fsname, fs_table[fs_num].fsname))
	    break;
    }
    if (fs_num >= sizeof(fs_table) / sizeof(struct fs_entry)) {
	return (EINVAL);
    }

    *fstype = fs_num;
    return EOK;
}

/* mount_fs
 *
 */
W
fs_mount(const ID device,
	 vnode_t * mountpoint, W option, W fstype)
{
    vfs_t *newfs;
    vnode_t *newip;
    vfs_operation_t *fsp;
    W err;

    if ((fstype < 0) || (fstype >= sizeof(fs_table) / sizeof(struct fs_entry))) {
	log_debug("fs: mount unknown fstype %d\n", fstype);
	return (EINVAL);
    }

    /* ファイルシステム情報の取り出し */
    fsp = fs_table[fstype].fsops;

    if (rootfs) {
	/* 既に mount されていないかどうかのチェック */
	newfs = rootfs;
	do {
	    if (newfs->device.channel == device) {
		return (EBUSY);
	    }
	    newfs = getFsParent(list_next(&(newfs->bros)));
	} while (newfs != rootfs);

	newip = vnodes_create();
	if (newip == NULL)
	    return (E_NOMEM);

    } else
	newip = mountpoint;

    newfs = alloc_fs();
    if (newfs == NULL)
	err = ENOMEM;

    else {
	err = fsp->mount(device, newfs, newip);
	if (err)
	    dealloc_fs(newfs);
    }

    if (err) {
	if (rootfs) {
	    vnodes_remove(newip);
	}

	return (err);
    }

    /* mount されるファイルシステムの root ディレクトリの登録 */
    newip->fs = newfs;
    newfs->root = newip;
    newfs->operations = *fsp;

    /* ファイルシステムのリストへ登録 */
    if (rootfs) {
	list_append(&(rootfs->bros), &(newfs->bros));

	/* mount point に coverfile を設定 */
	mountpoint->covered = newip;
	newfs->origin = mountpoint;

    } else
	rootfs = newfs;

    vnodes_append(newip);

    return (EOK);
}


/* unmount_fs
 *
 */
W fs_unmount(UW device)
{
    vfs_t *fsp;

    /* device から fsp を検索 */
    fsp = rootfs;
    do {
	if (fsp->device.channel == device)
	    break;
	fsp = getFsParent(list_next(&(fsp->bros)));
    }
    while (fsp != rootfs);
    if (fsp == rootfs) {
	/* 見付からなかったか，root file system だった場合 */
	return (EINVAL);
    }

    if (fsp->root->refer_count > 1) {
	return (EBUSY);
    }

    if (!list_is_empty(&(fsp->vnodes))) {
	/* マウントポイント以下のファイル/ディレクトリが使われている
	 * BUSY のエラーで返す
	 */
	return (EBUSY);
    }

    /* ファイルシステム情報を解放する */
    fsp->operations.unmount(fsp);

    /* マウントポイントを解放する */
    fsp->origin->covered = NULL;
    vnodes_remove(fsp->origin);

    /* FS list から除外 */
    dealloc_fs(fsp);

    return (EOK);
}


/* fs_link_file -
 *
 */
W
fs_link_file(W procid, B * src, B * dst, struct permission * acc)
{
    char parent_path[NAME_MAX + 1];
    vnode_t *startip;
    vnode_t *srcip, *parent_ip;
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

    error_no = vfs_walk(startip, src, O_RDONLY, acc, &srcip);
    if (error_no) {
	return (error_no);
    }

    /* リンク元がディレクトリならエラー */
    if ((srcip->mode & S_IFMT) == S_IFDIR) {
	vnodes_remove(srcip);
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

    parent_length = copy_path(parent_path, dst, startip, &parent_ip);
    if (parent_length > 0) {
	error_no =
	    vfs_walk(startip, parent_path, O_RDWR, acc, &parent_ip);
	if (error_no) {
	    return (error_no);
	}
    }
    parent_length += 1;

    /* ファイルシステムを跨ぐリンクにならないことをチェックする */
    if (srcip->fs != parent_ip->fs) {
	vnodes_remove(parent_ip);
	vnodes_remove(srcip);
	return (EXDEV);
    }

    /* リンク先にファイルが存在していたらエラー */
    vnode_t *ip;
    error_no = vfs_walk(parent_ip, &dst[parent_length], O_RDONLY, acc, &ip);
    if (error_no == EOK) {
	vnodes_remove(ip);
	error_no = EEXIST;
    } else {
	/* 各ファイルシステムの link 関数を呼び出す */
	error_no = parent_ip->fs->operations.link(parent_ip, &dst[parent_length], srcip);
    }
    vnodes_remove(parent_ip);
    vnodes_remove(srcip);
    if (error_no) {
	return (error_no);
    }
    return (EOK);
}


/* --------=========== 細々とした関数群 ================--------- */

static W copy_path(char * parent_path, char * path, vnode_t * startip,
		vnode_t ** parent_ip)
{
    W len;

    for (len = strlen(path); len >= 0; len--) {
	if (path[len] == '/') {
	    strncpy(parent_path, path, NAME_MAX + 1);
	    parent_path[NAME_MAX] = '\0';
	    break;
	}
    }

    if (len < 0) {
	*parent_ip = startip;
    } else if (len == 0) {
	*parent_ip = rootfile;
    }

    return len;
}
