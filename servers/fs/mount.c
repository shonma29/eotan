/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * $Log: mount.c,v $
 * Revision 1.3  1999/03/24 05:13:19  monaka
 * Source file cleaning for avoid warnings.
 *
 * Revision 1.2  1999/03/24 05:12:52  monaka
 * Source file cleaning for avoid warnings.
 *
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include <fcntl.h>
#include <core/options.h>
#include <nerve/kcall.h>
#include <sys/stat.h>
#include "fs.h"
#include "api.h"

/* if_mount - ファイルシステムをマウントする
 *
 * 引数:
 *	devname		マウントするデバイス名
 *	dirname		マウントするディレクトリ名
 *	fstype		ファイルシステムのタイプ名
 *	option		オプション
 *
 * Note:
 *	このシステムコールは、POSIX では定義されていない。
 *
 */
int if_mount(fs_request *req)
{
    W error_no;
    W fstype;
    vnode_t *startip;
    vnode_t *mountpoint, *device;
    struct permission acc;
    ID caller = get_rdv_tid(req->rdvno);

    error_no = proc_get_permission(req->packet.procid, &acc);
    if (error_no)
	return error_no;

    if (acc.uid != ROOT_UID)
      return EACCES;

    error_no = kcall->region_copy(caller,
		     (UB*)(req->packet.args.arg1),
		     sizeof(req->buf) - 1, req->buf);
    if (error_no < 0) {
	/* ファイルシステムタイプのコピーエラー */
	if (error_no == E_PAR)
	    return EINVAL;
	else
	    return EFAULT;
    }
    req->buf[MAX_NAMELEN] = '\0';
    error_no = find_fs((UB*)(req->buf), &fstype);
    if (error_no)
	return error_no;

    error_no = session_get_path(&startip, req->packet.procid,
		     caller, (UB*)(req->packet.args.arg4),
		     (UB*)(req->buf));
    if (error_no)
	return error_no;

    error_no = fs_open_file(req->buf, O_RDWR, 0, &acc, startip, &device);
    if (error_no)
	return error_no;

    /* block device かどうかのチェック */
    if ((device->mode & S_IFMT) != S_IFBLK) {
	dealloc_inode(device);
	return EINVAL;
    }

    error_no = session_get_path(&startip, req->packet.procid,
		     caller, (UB*)(req->packet.args.arg2),
		     (UB*)(req->buf));
    if (error_no) {
	dealloc_inode(device);
	return error_no;
    }

    error_no = fs_open_file(req->buf, O_RDWR, 0, &acc, startip, &mountpoint);
    if (error_no) {
	dealloc_inode(device);
	return error_no;
    }

    if (mountpoint->refer_count > 1) {
	dealloc_inode(device);
	dealloc_inode(mountpoint);
	return EBUSY;
    }

    if ((mountpoint->mode & S_IFMT) != S_IFDIR) {
	dealloc_inode(device);
	dealloc_inode(mountpoint);
	return ENOTDIR;
    }

    error_no =
	fs_mount(device->dev, mountpoint, req->packet.args.arg3, fstype);

    if (error_no == EOK) {
	dealloc_inode(device);
	put_response(req->rdvno, EOK, 0, 0);
	return EOK;
    }

    dealloc_inode(device);
    dealloc_inode(mountpoint);
    return error_no;
}

int if_unmount(fs_request *req)
{
    W error_no;
    UW device = 0;
    vnode_t *startip;
    vnode_t *umpoint;
    struct permission acc;

    error_no = session_get_path(&startip, req->packet.procid,
		     get_rdv_tid(req->rdvno), (UB*)(req->packet.args.arg1),
		     (UB*)(req->buf));
    if (error_no)
	return error_no;

    error_no = proc_get_permission(req->packet.procid, &acc);
    if (error_no)
	return error_no;

    if (acc.uid != ROOT_UID)
	return EACCES;

    error_no = fs_open_file(req->buf, O_RDWR, 0, &acc, startip, &umpoint);
    if (error_no)
	return error_no;

    error_no = EOK;
    switch (umpoint->mode & S_IFMT) {
    case S_IFDIR:
	device = umpoint->fs->device.channel;
	break;
    case S_IFREG:
        error_no = ENOTDIR;
	break;
      default:
	error_no = EINVAL;
	break;
    }

    dealloc_inode(umpoint);
    if (error_no == EOK) {
	error_no = fs_unmount(device);
    }
    if (error_no)
	return error_no;

    put_response(req->rdvno, EOK, 0, 0);
    return EOK;
}
