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
    struct inode *startip;
    struct inode *mountpoint, *device;
    struct permission acc;
    ID caller = get_rdv_tid(req->rdvno);

    error_no = proc_get_permission(req->packet.procid, &acc);
    if (error_no)
	return error_no;

    if (acc.uid != SU_UID)
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

    error_no = kcall->region_copy(caller,
		     (UB*)(req->packet.args.arg4),
		     sizeof(req->buf) - 1, req->buf);
    if (error_no < 0) {
	/* デバイスファイルのパス名のコピーエラー */
	if (error_no == E_PAR)
	    return EINVAL;
	else
	    return EFAULT;
    }
    req->buf[MAX_NAMELEN] = '\0';

    /* デバイスのオープン */
    if (req->buf[0] != '/') {
	error_no = proc_get_cwd(req->packet.procid, &startip);
	if (error_no)
	    return error_no;

    } else {
	startip = rootfile;
    }

    error_no = fs_open_file(req->buf, O_RDWR, 0, &acc, startip, &device);
    if (error_no)
	return error_no;

    /* block device かどうかのチェック */
    if ((device->i_mode & S_IFMT) != S_IFBLK) {
	fs_close_file(device);
	return EINVAL;
    }

    error_no = kcall->region_copy(caller,
		     (UB*)(req->packet.args.arg2),
		     sizeof(req->buf) - 1, req->buf);

    if (error_no < 0) {
	/* mount 先のパス名のコピーエラー */
	if (error_no == E_PAR) {
	    fs_close_file(device);
	    return EINVAL;
	}
	else {
	    fs_close_file(device);
	    return EFAULT;
	}
    }
    req->buf[MAX_NAMELEN] = '\0';

    /* マウントポイントのオープン */
    if (req->buf[0] != '/') {
	error_no = proc_get_cwd(req->packet.procid, &startip);
	if (error_no) {
	    fs_close_file(device);
	    return error_no;
	}
    } else {
	startip = rootfile;
    }

    error_no = fs_open_file(req->buf, O_RDWR, 0, &acc, startip, &mountpoint);
    if (error_no) {
	fs_close_file(device);
	return error_no;
    }

    if (mountpoint->i_refcount > 1) {
	fs_close_file(device);
	fs_close_file(mountpoint);
	return EBUSY;
    }

    if ((mountpoint->i_mode & S_IFMT) != S_IFDIR) {
	fs_close_file(device);
	fs_close_file(mountpoint);
	return ENOTDIR;
    }

    error_no =
	fs_mount(device->i_dev, mountpoint, req->packet.args.arg3, fstype);

    if (error_no == EOK) {
	fs_close_file(device);
	put_response(req->rdvno, EOK, 0, 0);
	return EOK;
    }

    fs_close_file(device);
    fs_close_file(mountpoint);
    return error_no;
}

int if_unmount(fs_request *req)
{
    W error_no;
    UW device = 0;
    struct inode *startip;
    struct inode *umpoint;
    struct permission acc;

    error_no = kcall->region_copy(get_rdv_tid(req->rdvno),
		     (UB*)(req->packet.args.arg1),
		     sizeof(req->buf) - 1, req->buf);

    if (error_no < 0) {
	/* mount 先/special file のパス名のコピーエラー */
	if (error_no == E_PAR)
	    return EINVAL;
	else
	    return EFAULT;
    }
    req->buf[MAX_NAMELEN] = '\0';

    /* アンマウントポイントのオープン */
    if (req->buf[0] != '/') {
	error_no = proc_get_cwd(req->packet.procid, &startip);
	if (error_no)
	    return error_no;

    } else {
	startip = rootfile;
    }

    error_no = proc_get_permission(req->packet.procid, &acc);
    if (error_no)
	return error_no;

    if (acc.uid != SU_UID)
	return EACCES;

    error_no = fs_open_file(req->buf, O_RDWR, 0, &acc, startip, &umpoint);
    if (error_no)
	return error_no;

    error_no = EOK;
    switch (umpoint->i_mode & S_IFMT) {
    case S_IFDIR:
	device = umpoint->i_fs->device;
	break;
    case S_IFREG:
        error_no = ENOTDIR;
	break;
      default:
	error_no = EINVAL;
	break;
    }

    fs_close_file(umpoint);
    if (error_no == EOK) {
	error_no = fs_unmount(device);
    }
    if (error_no)
	return error_no;

    put_response(req->rdvno, EOK, 0, 0);
    return EOK;
}
