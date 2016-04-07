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
void if_mount(fs_request *req)
{
    W error_no;
    B devname[MAX_NAMELEN + 1];
    B fstype[MAX_NAMELEN + 1];
    struct inode *startip;
    struct inode *mountpoint, *device;
    struct permission acc;
    ID caller = get_rdv_tid(req->rdvno);

    error_no = kcall->region_copy(caller,
		     (UB*)(req->packet.args.arg4),
		     sizeof(devname) - 1, devname);
    if (error_no < 0) {
	/* デバイスファイルのパス名のコピーエラー */
	if (error_no == E_PAR)
	    put_response(req->rdvno, EINVAL, -1, 0);
	else
	    put_response(req->rdvno, EFAULT, -1, 0);

	return;
    }
    devname[MAX_NAMELEN] = '\0';
    error_no = kcall->region_copy(caller,
		     (UB*)(req->packet.args.arg2),
		     sizeof(req->buf) - 1, req->buf);

    if (error_no < 0) {
	/* mount 先のパス名のコピーエラー */
	if (error_no == E_PAR) {
	    put_response(req->rdvno, EINVAL, -1, 0);
	} else {
	    put_response(req->rdvno, EFAULT, -1, 0);
	}
	return;
    }
    req->buf[MAX_NAMELEN] = '\0';

    error_no = kcall->region_copy(caller,
		     (UB*)(req->packet.args.arg1),
		     sizeof(fstype) - 1, fstype);
    if (error_no < 0) {
	/* ファイルシステムタイプのコピーエラー */
	if (error_no == E_PAR)
	    put_response(req->rdvno, EINVAL, -1, 0);
	else
	    put_response(req->rdvno, EFAULT, -1, 0);

	return;
    }
    fstype[MAX_NAMELEN] = '\0';

    /* デバイスのオープン */
    if (*devname != '/') {
	error_no = proc_get_cwd(req->packet.procid, &startip);
	if (error_no) {
	    put_response(req->rdvno, error_no, -1, 0);
	    return;
	}
    } else {
	startip = rootfile;
    }

    error_no = proc_get_permission(req->packet.procid, &acc);
    if (error_no) {
	put_response(req->rdvno, error_no, -1, 0);
	return;
    }
    if (acc.uid != SU_UID) {
      put_response(req->rdvno, EACCES, -1, 0);
      return;
    }

    error_no = fs_open_file(devname, O_RDWR, 0, &acc, startip, &device);
    if (error_no) {
	put_response(req->rdvno, error_no, -1, 0);
	return;
    }

    /* block device かどうかのチェック */
    if ((device->i_mode & S_IFMT) != S_IFBLK) {
	fs_close_file(device);
	put_response(req->rdvno, EINVAL, -1, 0);
	return;
    }

    /* マウントポイントのオープン */
    if (req->buf[0] != '/') {
	error_no = proc_get_cwd(req->packet.procid, &startip);
	if (error_no) {
	    fs_close_file(device);
	    put_response(req->rdvno, error_no, -1, 0);
	    return;
	}
    } else {
	startip = rootfile;
    }

    error_no = fs_open_file(req->buf, O_RDWR, 0, &acc, startip, &mountpoint);
    if (error_no) {
	put_response(req->rdvno, error_no, -1, 0);
	fs_close_file(device);
	return;
    }

    if (mountpoint->i_refcount > 1) {
	put_response(req->rdvno, EBUSY, -1, 0);
	fs_close_file(device);
	fs_close_file(mountpoint);
	return;
    }

    if ((mountpoint->i_mode & S_IFMT) != S_IFDIR) {
	put_response(req->rdvno, ENOTDIR, -1, 0);
	fs_close_file(device);
	fs_close_file(mountpoint);
	return;
    }

    error_no =
	mount_fs(device, mountpoint, req->packet.args.arg3, fstype);

    if (error_no == EOK) {
	put_response(req->rdvno, EOK, 0, 0);
	fs_close_file(device);
	return;
    }

    put_response(req->rdvno, error_no, -1, 0);
    fs_close_file(device);
    fs_close_file(mountpoint);
}

void if_unmount(fs_request *req)
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
	if (error_no == E_PAR) {
	    put_response(req->rdvno, EINVAL, -1, 0);
	} else {
	    put_response(req->rdvno, EFAULT, -1, 0);
	}
	return;
    }
    req->buf[MAX_NAMELEN] = '\0';

    /* アンマウントポイントのオープン */
    if (req->buf[0] != '/') {
	error_no = proc_get_cwd(req->packet.procid, &startip);
	if (error_no) {
	    put_response(req->rdvno, error_no, -1, 0);
	    return;
	}
    } else {
	startip = rootfile;
    }

    error_no = proc_get_permission(req->packet.procid, &acc);
    if (error_no) {
	put_response(req->rdvno, error_no, -1, 0);
	return;
    }
    if (acc.uid != SU_UID) {
	put_response(req->rdvno, EACCES, -1, 0);
	return;
    }

    error_no = fs_open_file(req->buf, O_RDWR, 0, &acc, startip, &umpoint);
    if (error_no) {
	put_response(req->rdvno, error_no, -1, 0);
	return;
    }

    error_no = EOK;
    switch (umpoint->i_mode & S_IFMT) {
    case S_IFDIR:
	device = umpoint->i_device;
	break;
    case S_IFBLK:
	device = umpoint->i_dev;
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
	error_no = unmount_fs(device);
    }
    if (error_no) {
	put_response(req->rdvno, error_no, -1, 0);
	return;
    }
    put_response(req->rdvno, EOK, 0, 0);
}