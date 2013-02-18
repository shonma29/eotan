/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * $Log: access.c,v $
 * Revision 1.3  2000/05/06 03:54:22  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.2  1999/07/10 10:45:34  naniwa
 * minor fix
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include <fcntl.h>
#include <string.h>
#include <utime.h>
#include "fs.h"

W psc_access_f(RDVNO rdvno, struct posix_request *req)
{
    B pathname[MAX_NAMELEN];
    W fileid;
    W errno;
    struct inode *startip;
    struct inode *newip;
    struct access_info acc;
    W euid;
    W accmode;



    errno = proc_alloc_fileid(req->procid, &fileid);
    if (errno) {
	/* メモリ取得エラー */
	put_response(rdvno, ENOMEM, -1, 0);
	return (FALSE);
    }
    memset(pathname, 0, req->param.par_access.pathlen + 1);

    /* パス名をユーザプロセスから POSIX サーバにコピーする。
     */
    errno =
	vget_reg(req->caller, req->param.par_access.path,
		 req->param.par_access.pathlen, pathname);
    if (errno) {
	/* パス名のコピーエラー */
	if (errno == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return (FALSE);
    }
#ifdef notdef
    printf("psc_access_f: open file path = %s\n", pathname);
#endif
    if (*pathname != '/') {
	errno = proc_get_cwd(req->procid, &startip);
	if (errno) {
	    put_response(rdvno, errno, -1, 0);
	    return (FALSE);
	}
    } else {
	startip = rootfile;
    }
    errno = proc_get_uid(req->procid, &(acc.uid));
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }
    errno = proc_get_euid(req->procid, &euid);
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }

    errno = proc_get_gid(req->procid, &(acc.gid));
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }

    errno = fs_open_file(pathname,
			 req->param.par_open.oflag,
			 req->param.par_open.mode, &acc, startip, &newip);
    if (errno) {
#ifdef notdef
	printf("open systemcall: Not found entry.\n");
#endif
	/* ファイルがオープンできない */
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }

    accmode = req->param.par_access.accflag;
    if (newip->i_uid == acc.uid) {
	accmode |= (req->param.par_access.accflag << 6);
    }
    if (newip->i_gid == acc.gid) {
	accmode |= (req->param.par_access.accflag << 3);
    }

    errno = fs_close_file(newip);
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }

    /* アクセス権限のチェック */
    if ((acc.uid == 0) || (euid == 0)) {
	/* root ユーザの場合には、無条件で成功とする */
	put_response(rdvno, EOK, 0, 0);
	return (TRUE);
    } else if ((newip->i_mode & accmode) == 0) {
	put_response(rdvno, EACCESS, -1, 0);
	return (FALSE);
    }

    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}

W psc_chmod_f(RDVNO rdvno, struct posix_request *req)
{
    B path[MAX_NAMELEN];
    struct inode *startip;
    struct inode *ipp;
    struct access_info acc;
    W err;

    if (vget_reg(req->caller, req->param.par_chmod.path,
		 req->param.par_chmod.pathlen + 1, path)) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    if (*path == '/') {
	/* 絶対パスによる指定 */
	startip = rootfile;
    } else {
	if (proc_get_cwd(req->procid, &startip)) {
	    put_response(rdvno, EINVAL, -1, 0);
	    return (FALSE);
	}
    }

    if (proc_get_euid(req->procid, &acc.uid)) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    if (proc_get_egid(req->procid, &acc.gid)) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }
    err = fs_lookup(startip, path, O_RDWR, 0, &acc, &ipp);
    if (err) {
	put_response(rdvno, ENOENT, -1, 0);
	return (FALSE);
    }

    ipp->i_mode = (ipp->i_mode & S_IFMT) | req->param.par_chmod.mode;
    ipp->i_ctime = get_system_time(NULL);
    ipp->i_dirty = 1;

    /* fs_close_file で行う処理 */
    if (fs_sync_file(ipp)) {
	put_response(rdvno, EINVAL, -1, 0);
	dealloc_inode(ipp);
	return (FALSE);
    }

    dealloc_inode(ipp);
    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}

W psc_chown_f(RDVNO rdvno, struct posix_request *req)
{
    B path[MAX_NAMELEN];
    struct inode *startip;
    struct inode *ipp;
    struct access_info acc;
    W err;

    if (vget_reg(req->caller, req->param.par_chown.path,
		 req->param.par_chown.pathlen + 1, path)) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    if (*path == '/') {
	/* 絶対パスによる指定 */
	startip = rootfile;
    } else {
	if (proc_get_cwd(req->procid, &startip)) {
	    put_response(rdvno, EINVAL, -1, 0);
	    return (FALSE);
	}
    }

    if (proc_get_euid(req->procid, &acc.uid)) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    if (proc_get_egid(req->procid, &acc.gid)) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    err = fs_lookup(startip, path, O_RDWR, 0, &acc, &ipp);
    if (err) {
	put_response(rdvno, ENOENT, -1, 0);
	return (FALSE);
    }

    ipp->i_uid = req->param.par_chown.uid;
    ipp->i_gid = req->param.par_chown.gid;
    ipp->i_ctime = get_system_time(NULL);
    ipp->i_dirty = 1;

    /* fs_close_file で行う処理 */
    if (fs_sync_file(ipp)) {
	put_response(rdvno, EINVAL, -1, 0);
	dealloc_inode(ipp);
	return (FALSE);
    }

    dealloc_inode(ipp);
    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}

/* psc_fstat_f - ファイルの情報を返す
 */
W psc_fstat_f(RDVNO rdvno, struct posix_request *req)
{
    struct file *fp;
    W errno;
    struct stat st;

    errno = proc_get_file(req->procid, req->param.par_fstat.fileid, &fp);
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }

    if (fp == 0) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    } else if (fp->f_inode == 0) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    st.st_dev = fp->f_inode->i_device;
    st.st_ino = fp->f_inode->i_index;
    st.st_mode = fp->f_inode->i_mode;
    st.st_nlink = fp->f_inode->i_link;
    st.st_size = fp->f_inode->i_size;
    st.st_uid = fp->f_inode->i_uid;
    st.st_gid = fp->f_inode->i_gid;
    st.st_rdev = fp->f_inode->i_dev;
    if (fp->f_inode->i_fs == NULL) {
      printk("FSTAT: illegal i_fs\n");
      st.st_blksize = 512;
    }
    else {
      st.st_blksize = fp->f_inode->i_fs->fs_blksize;
    }
    st.st_blocks = ROUNDUP(st.st_size, st.st_blksize) / st.st_blksize;
    st.st_atime = fp->f_inode->i_atime;
    st.st_mtime = fp->f_inode->i_mtime;
    st.st_ctime = fp->f_inode->i_ctime;

    errno =
	vput_reg(req->caller, req->param.par_fstat.st, sizeof(struct stat),
		 &st);
    if (errno) {
	put_response(rdvno, EINVAL, 0, 0);
	return (FALSE);
    }

    put_response(rdvno, EOK, 0, 0);

    return (TRUE);
}

W
psc_statfs_f (RDVNO rdvno, struct posix_request *req)
{
  struct statfs	result;
  ER		errno;

  errno = fs_statfs (req->param.par_statfs.device, &result);
  if (errno)
    {
      put_response (rdvno, errno, -1, 0);    
      return (FALSE);
    }

#ifdef notdef
  printf ("result.f_type = 0x%x\n", result.f_type);
  printf ("result.f_blksize = 0x%x\n", result.f_bsize);
#endif
  
  errno = vput_reg (req->caller, req->param.par_statfs.fsp, sizeof (struct statfs), &result);
  if (errno)
    {
      put_response (rdvno, EFAULT, -1, 0);
      return (FALSE);
    }
  put_response (rdvno, EOK, 0, 0);
  return (TRUE);
}

W psc_utime_f(RDVNO rdvno, struct posix_request *req)
{
    B pathname[MAX_NAMELEN];
    ER errno = EOK;
    struct utimbuf tb;
    struct inode *startip;
    struct inode *ipp;
    struct access_info acc;

    errno = vget_reg(req->caller, req->param.par_utime.path,
		     req->param.par_utime.pathlen + 1, pathname);
    if (errno) {
	/* パス名のコピーエラー */
	if (errno == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return (FALSE);
    }

    errno = vget_reg(req->caller, req->param.par_utime.buf,
		     sizeof(struct utimbuf), &tb);
    if (errno) {
	if (errno == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return (FALSE);
    }

    if (*pathname != '/') {
	errno = proc_get_cwd(req->procid, &startip);
	if (errno) {
	    put_response(rdvno, errno, -1, 0);
	    return (FALSE);
	}
    } else {
	startip = rootfile;
    }
    errno = proc_get_euid(req->procid, &(acc.uid));
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }

    errno = proc_get_egid(req->procid, &(acc.gid));
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }

    errno = fs_lookup(startip, pathname, O_RDWR, 0, &acc, &ipp);
    if (errno) {
	put_response(rdvno, ENOENT, -1, 0);
	return (FALSE);
    }

    ipp->i_atime = tb.actime;
    ipp->i_mtime = tb.modtime;
    ipp->i_dirty = 1;

    /* fs_close_file で行う処理 */
    if (fs_sync_file(ipp)) {
	put_response(rdvno, EINVAL, -1, 0);
	dealloc_inode(ipp);
	return (FALSE);
    }

    dealloc_inode(ipp);

    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}
