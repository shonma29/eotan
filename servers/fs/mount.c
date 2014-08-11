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
#include "fs.h"

/* psc_mount_f - ファイルシステムをマウントする
 *
 * 引数:
 *	devnamelen	マウントするデバイス名の長さ
 *	devname		マウントするデバイス名
 *	dirnamelen	マウントするディレクトリ名の長さ
 *	dirname		マウントするディレクトリ名
 *	fstypelen	ファイルシステムのタイプ名の長さ
 *	fstype		ファイルシステムのタイプ名
 *	option		オプション
 *
 * Note:
 *	このシステムコールは、POSIX では定義されていない。
 *
 */
void psc_mount_f(RDVNO rdvno, struct posix_request *req)
{
    W error_no;
    B dirname[MAX_NAMELEN];
    B devname[MAX_NAMELEN];
    B fstype[MAX_NAMELEN];
    struct inode *startip;
    struct inode *mountpoint, *device;
    struct permission acc;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;
    ID caller = get_rdv_tid(rdvno);

    error_no = kcall->region_get(caller,
		     req->param.par_mount.devname,
		     req->param.par_mount.devnamelen + 1, devname);
    if (error_no) {
	/* デバイスファイルのパス名のコピーエラー */
	if (error_no == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return;
    }
    error_no = kcall->region_get(caller,
		     req->param.par_mount.dirname,
		     req->param.par_mount.dirnamelen + 1, dirname);

    if (error_no) {
	/* mount 先のパス名のコピーエラー */
	if (error_no == E_PAR) {
	    put_response(rdvno, EINVAL, -1, 0);
	} else {
	    put_response(rdvno, EFAULT, -1, 0);
	}
	return;
    }

    error_no = kcall->region_get(caller,
		     req->param.par_mount.fstype,
		     req->param.par_mount.fstypelen + 1, fstype);
    if (error_no) {
	/* ファイルシステムタイプのコピーエラー */
	if (error_no == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return;
    }

    /* デバイスのオープン */
    if (*devname != '/') {
	error_no = proc_get_cwd(req->procid, &startip);
	if (error_no) {
	    put_response(rdvno, error_no, -1, 0);
	    return;
	}
    } else {
	startip = rootfile;
    }

    error_no = proc_get_permission(req->procid, &acc);
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return;
    }
    if (acc.uid != SU_UID) {
      put_response(rdvno, EACCES, -1, 0);
      return;
    }

    error_no = fs_open_file(devname, O_RDWR, 0, &acc, startip, &device);
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return;
    }

    /* block device かどうかのチェック */
    if ((device->i_mode & S_IFMT) != S_IFBLK) {
	fs_close_file(device);
	put_response(rdvno, EINVAL, -1, 0);
	return;
    }

    /* マウントポイントのオープン */
    if (*dirname != '/') {
	error_no = proc_get_cwd(req->procid, &startip);
	if (error_no) {
	    fs_close_file(device);
	    put_response(rdvno, error_no, -1, 0);
	    return;
	}
    } else {
	startip = rootfile;
    }

    error_no = fs_open_file(dirname, O_RDWR, 0, &acc, startip, &mountpoint);
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	fs_close_file(device);
	return;
    }

    if (mountpoint->i_refcount > 1) {
	put_response(rdvno, EBUSY, -1, 0);
	fs_close_file(device);
	fs_close_file(mountpoint);
	return;
    }

    if ((mountpoint->i_mode & S_IFMT) != S_IFDIR) {
	put_response(rdvno, ENOTDIR, -1, 0);
	fs_close_file(device);
	fs_close_file(mountpoint);
	return;
    }

    error_no =
	mount_fs(device, mountpoint, req->param.par_mount.option, fstype);

    if (error_no == EOK) {
	put_response(rdvno, EOK, 0, 0);
	fs_close_file(device);
	return;
    }

    put_response(rdvno, error_no, -1, 0);
    fs_close_file(device);
    fs_close_file(mountpoint);
}

void psc_umount_f(RDVNO rdvno, struct posix_request *req)
{
    W error_no;
    UW device = 0;
    B dirname[MAX_NAMELEN];
    struct inode *startip;
    struct inode *umpoint;
    struct permission acc;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    error_no = kcall->region_get(get_rdv_tid(rdvno),
		     req->param.par_umount.dirname,
		     req->param.par_umount.dirnamelen + 1, dirname);

    if (error_no) {
	/* mount 先/special file のパス名のコピーエラー */
	if (error_no == E_PAR) {
	    put_response(rdvno, EINVAL, -1, 0);
	} else {
	    put_response(rdvno, EFAULT, -1, 0);
	}
	return;
    }

    /* アンマウントポイントのオープン */
    if (*dirname != '/') {
	error_no = proc_get_cwd(req->procid, &startip);
	if (error_no) {
	    put_response(rdvno, error_no, -1, 0);
	    return;
	}
    } else {
	startip = rootfile;
    }

    error_no = proc_get_permission(req->procid, &acc);
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return;
    }
    if (acc.uid != SU_UID) {
	put_response(rdvno, EACCES, -1, 0);
	return;
    }

    error_no = fs_open_file(dirname, O_RDWR, 0, &acc, startip, &umpoint);
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
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
	error_no = umount_fs(device);
    }
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return;
    }
    put_response(rdvno, EOK, 0, 0);
}
