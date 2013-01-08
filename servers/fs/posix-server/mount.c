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

#include "posix.h"

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
W psc_mount_f(RDVNO rdvno, struct posix_request *req)
{
    W errno;
#ifdef USE_ALLOCA
    B *dirname, *devname;
#else
    B dirname[MAX_NAMELEN];
    B devname[MAX_NAMELEN];
#endif
    B fstype[MAX_NAMELEN];
    struct inode *startip;
    struct inode *mountpoint, *device;
    struct access_info acc;

#ifdef USE_ALLOCA
    devname =
	(B *) alloca(sizeof(B) * (req->param.par_mount.devnamelen + 1));
    if (devname == NULL) {
	put_response(rdvno, req, EP_NOMEM, 0, 0, 0);
	return (FAIL);
    }
#endif

    errno = vget_reg(req->caller,
		     req->param.par_mount.devname,
		     req->param.par_mount.devnamelen + 1, devname);
    if (errno) {
	/* デバイスファイルのパス名のコピーエラー */
	if (errno == E_PAR)
	    put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	else
	    put_response(rdvno, req, EP_FAULT, -1, 0, 0);

	return (FAIL);
    }
#ifdef USE_ALLOCA
    dirname =
	(B *) alloca(sizeof(B) * (req->param.par_mount.dirnamelen + 1));
    if (dirname == NULL) {
	put_response(rdvno, req, EP_NOMEM, 0, 0, 0);
	return (FAIL);
    }
#endif
    errno = vget_reg(req->caller,
		     req->param.par_mount.dirname,
		     req->param.par_mount.dirnamelen + 1, dirname);

    if (errno) {
	/* mount 先のパス名のコピーエラー */
	if (errno == E_PAR) {
	    put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	} else {
	    put_response(rdvno, req, EP_FAULT, -1, 0, 0);
	}
	return (FAIL);
    }

    errno = vget_reg(req->caller,
		     req->param.par_mount.fstype,
		     req->param.par_mount.fstypelen + 1, fstype);
    if (errno) {
	/* ファイルシステムタイプのコピーエラー */
	if (errno == E_PAR)
	    put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	else
	    put_response(rdvno, req, EP_FAULT, -1, 0, 0);

	return (FAIL);
    }

    /* デバイスのオープン */
    if (*devname != '/') {
	errno = proc_get_cwd(req->procid, &startip);
	if (errno) {
	    put_response(rdvno, req, errno, -1, 0, 0);
	    return (FAIL);
	}
    } else {
	startip = rootfile;
    }

    errno = proc_get_euid(req->procid, &(acc.uid));
    if (errno) {
	put_response(rdvno, req, errno, -1, 0, 0);
	return (FAIL);
    }
    if (acc.uid != SU_UID) {
      put_response(rdvno, req, EP_ACCESS, -1, 0, 0);
      return (FAIL);
    }

    errno = proc_get_egid(req->procid, &(acc.gid));
    if (errno) {
	put_response(rdvno, req, errno, -1, 0, 0);
	return (FAIL);
    }
    errno = fs_open_file(devname, O_RDWR, 0, &acc, startip, &device);
    if (errno) {
	put_response(rdvno, req, errno, -1, 0, 0);
	return (FAIL);
    }

    /* block device かどうかのチェック */
    if ((device->i_mode & FS_FMT_MSK) != FS_FMT_BDEV) {
	fs_close_file(device);
	put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	return (FAIL);
    }

    /* マウントポイントのオープン */
    if (*dirname != '/') {
	errno = proc_get_cwd(req->procid, &startip);
	if (errno) {
	    fs_close_file(device);
	    put_response(rdvno, req, errno, -1, 0, 0);
	    return (FAIL);
	}
    } else {
	startip = rootfile;
    }

    errno = fs_open_file(dirname, O_RDWR, 0, &acc, startip, &mountpoint);
    if (errno) {
	put_response(rdvno, req, errno, -1, 0, 0);
	fs_close_file(device);
	return (FAIL);
    }

    if (mountpoint->i_refcount > 1) {
	put_response(rdvno, req, EP_BUSY, -1, 0, 0);
	fs_close_file(device);
	fs_close_file(mountpoint);
	return (FAIL);
    }

    if ((mountpoint->i_mode & FS_FMT_MSK) != FS_FMT_DIR) {
	put_response(rdvno, req, EP_NOTDIR, -1, 0, 0);
	fs_close_file(device);
	fs_close_file(mountpoint);
	return (FAIL);
    }

    errno =
	mount_fs(device, mountpoint, req->param.par_mount.option, fstype);

    if (errno == EP_OK) {
	put_response(rdvno, req, EP_OK, 0, 0, 0);
	fs_close_file(device);
#ifdef notdef
	/* マウントポイントは，umount されるまで使用される */
	fs_close_file(mountpoint);
#endif
	return (SUCCESS);
    }

    put_response(rdvno, req, errno, -1, 0, 0);
    fs_close_file(device);
    fs_close_file(mountpoint);
    return (FAIL);
}
