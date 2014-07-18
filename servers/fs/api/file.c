/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/*
 * $Log: close.c,v $
 * Revision 1.3  2000/02/04 15:16:25  naniwa
 * minor fix
 *
 * Revision 1.2  1999/05/30 04:08:19  naniwa
 * modified to clear closed file slot
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include <device.h>
#include <fcntl.h>
#include <string.h>
#include <nerve/kcall.h>
#include "fs.h"

extern W sfs_open_device(ID device, W * rsize);
extern W sfs_read_device(ID device, B * buf, W start, W length, W * rlength);

static W control_device(ID device, struct posix_request *preq);

W psc_close_f(RDVNO rdvno, struct posix_request *req)
{
    struct file *fp;
    W err;
    ER sfs_close_device();

    err = proc_get_file(req->procid, req->param.par_close.fileid, &fp);
    if (err) {
	put_response(rdvno, err, -1, 0);
	return (FALSE);
    }

    if (fp->f_inode == NULL) {
	put_response(rdvno, EBADF, -1, 0);
	return (FALSE);
    }

    if (fp->f_inode->i_mode & S_IFCHR) {
	/* スペシャルファイルだった */
	/* デバイスに DEV_CLS メッセージを発信 */
	err = sfs_close_device(fp->f_inode->i_dev);
    }

    err = fs_close_file(fp->f_inode);
    if (err) {
	put_response(rdvno, err, -1, 0);
	return (FALSE);
    }

    fp->f_inode = NULL;
    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}

/* psc_dup_f - ファイル記述子の複製
 */
W
psc_dup_f (RDVNO rdvno, struct posix_request *req)
{
  W		error_no;
  struct file	*fp;
  W		newfileid;


  /* プロセスからファイル構造体へのポインタを取り出す
   */
  error_no = proc_get_file (req->procid, req->param.par_dup.fileid, &fp);
  if (error_no)
    {
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }


  if (fp->f_inode == NULL)
    {
      /* 複製するファイル記述子の番号がおかしかった
       */
      put_response (rdvno, EBADF, -1, 0);
      return (FALSE);
    }

  error_no = proc_alloc_fileid (req->procid, &newfileid);
  if (error_no)
    {
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }

  fp->f_inode->i_refcount++;
  error_no = proc_set_file (req->procid, newfileid, fp->f_omode, fp->f_inode);
  if (error_no)
    {
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }

  put_response (rdvno, EOK, newfileid, 0);
  return (TRUE);
}  

/* psc_dup2_f -ファイル記述子の複製
 */
W
psc_dup2_f (RDVNO rdvno, struct posix_request *req)
{
  W		error_no;
  struct file	*fp, *fp2;


  /* プロセスからファイル構造体へのポインタを取り出す
   */
  error_no = proc_get_file (req->procid, req->param.par_dup2.fileid1, &fp);
  if (error_no)
    {
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }


  if (fp->f_inode == NULL)
    {
      /* 複製するファイル記述子の番号がおかしい。
       * (ファイルをオープンしていない)
       */
      put_response (rdvno, EBADF, -1, 0);
      return (FALSE);
    }

  error_no = proc_get_file (req->procid, req->param.par_dup2.fileid2, &fp2);
  if (error_no) {
    put_response (rdvno, error_no, -1, 0);
    return (FALSE);
  }
  if (fp2->f_inode != NULL) {
    /* 既に open されている file id だった */
    error_no = fs_close_file (fp2->f_inode);
    if (error_no) {
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }
    fp2->f_inode = NULL;
  }
  fp->f_inode->i_refcount++;
  error_no = proc_set_file(req->procid, req->param.par_dup2.fileid2,
			fp->f_omode, fp->f_inode);
  if (error_no)
    {
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }

  put_response (rdvno, EOK, req->param.par_dup2.fileid2, 0);
  return (TRUE);
}  

/* ctl_device - デバイスにコントロールメッセージを送る
 *
 */
static W control_device(ID device, struct posix_request *preq)
{
    devmsg_t packet;
    W error_no;
    ID send_port;
    UW dd;
    ER_UINT rlength;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    error_no = get_device_info(device, &send_port, &dd);
    if (error_no) {
	return (error_no);
    }

    packet.req.header.msgtyp = DEV_CTL;
    packet.req.body.ctl_req.dd = dd;
    packet.req.body.ctl_req.cmd = (preq->param.par_fcntl.cmd >> 16)
	    & 0x0FFFF;
    packet.req.body.ctl_req.len = preq->param.par_fcntl.cmd & 0x0FFFF;

    if (packet.req.body.ctl_req.len == 0) {
	W *p = (W*)(packet.req.body.ctl_req.param);
	/* W に cast しているが、UB のまま代入したほうが良いかも */
	*p = (W) preq->param.par_fcntl.arg;
	packet.req.body.ctl_req.len = sizeof(W);
    } else {
	error_no = kcall->region_get(preq->caller, preq->param.par_fcntl.arg,
			 packet.req.body.ctl_req.len,
			 packet.req.body.ctl_req.param);
	if (error_no) {
	    dbg_printf("fctl: vget_reg error\n");
	    return (error_no);
	}
    }

    rlength = kcall->port_call(send_port, &packet, sizeof(packet.req));
    if (rlength < 0) {
	dbg_printf("cannot call port. %d\n", rlength);	/* */
	return (ENODEV);
    }

    return (packet.res.body.ctl_res.errinfo);
}

/* psc_fcntl_f - ファイルに対して特殊な操作を行う。
 */
W psc_fcntl_f(RDVNO rdvno, struct posix_request * req)
{
    W error_no;
    struct file *fp;
    ID device;

    error_no = proc_get_file(req->procid, req->param.par_fcntl.fileid, &fp);
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return (FALSE);
    } else if (fp == 0) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    } else if (fp->f_inode == 0) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    if (fp->f_inode->i_mode & S_IFCHR) {
	/* スペシャルファイルだった
	 */
	device = fp->f_inode->i_dev;

	/* send message to the device.
	 */
	error_no = control_device(device, req);
	if (error_no) {
	    put_response(rdvno, error_no, error_no, 0);
	    return (FALSE);
	} else {
	    put_response(rdvno, EOK, error_no, 0);
	    return (TRUE);
	}
    } else {
	/* とりあえず、サポートしていないというエラーで返す
	 */
	put_response(rdvno, ENOTSUP, 0, 0);
    }

    return (FALSE);
}

W psc_lseek_f(RDVNO rdvno, struct posix_request *req)
{
    struct file *fp;
    W error_no;

    error_no = proc_get_file(req->procid, req->param.par_lseek.fileid, &fp);
    if (error_no) {
	put_response(rdvno, error_no, -1, error_no);
	return (FALSE);
    }

    switch (req->param.par_lseek.mode) {
    case SEEK_SET:
	fp->f_offset = req->param.par_lseek.offset;
	break;

    case SEEK_CUR:
	fp->f_offset += req->param.par_lseek.offset;
	break;

    case SEEK_END:
	fp->f_offset = fp->f_inode->i_size + req->param.par_lseek.offset;
	break;

    default:
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    if (fp->f_offset < 0) {
	fp->f_offset = 0;
    }
    else if (fp->f_inode->i_mode & S_IFCHR) {
      if (fp->f_offset > fp->f_inode->i_size) {
	/* ブロックデバイスなど，サイズの制限のあるデバイスの場合 */
	fp->f_offset = fp->f_inode->i_size;
      }
    }
#ifdef notdef 
    else if (fp->f_offset > fp->f_inode->i_size) {
	/* デバイスドライバでない通常のファイルの場合 */
	/* EOF を越えた読み書きはまだサポートされていない */
	fp->f_offset = fp->f_inode->i_size;
    }
#endif

    put_response(rdvno, EOK, fp->f_offset, 0);

    return (TRUE);
}

/* psc_open_f - ファイルのオープン
 */
W psc_open_f(RDVNO rdvno, struct posix_request *req)
{
    B pathname[MAX_NAMELEN];
    W fileid;
    W error_no;
    struct inode *startip;
    struct inode *newip;
    struct access_info acc;
    W umask;
    W rsize;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    error_no = proc_alloc_fileid(req->procid, &fileid);
    if (error_no) {
	/* メモリ取得エラー */
	put_response(rdvno, ENOMEM, -1, 0);
	return (FALSE);
    }

    /* パス名をユーザプロセスから POSIX サーバのメモリ空間へコピーする。
     */
    error_no = kcall->region_get(req->caller, req->param.par_open.path,
		     req->param.par_open.pathlen + 1, pathname);
    if (error_no) {
	/* パス名のコピーエラー */
	if (error_no == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return (FALSE);
    }
#ifdef notdef
    dbg_printf("psc_open_f: open file path len = %d [%s]\n",
	       req->param.par_open.pathlen, pathname);
#endif
    if (*pathname != '/') {
	error_no = proc_get_cwd(req->procid, &startip);
	if (error_no) {
	    put_response(rdvno, error_no, -1, 0);
	    return (FALSE);
	}
    } else {
	startip = rootfile;
    }
    error_no = proc_get_uid(req->procid, &(acc.uid));
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return (FALSE);
    }

    error_no = proc_get_gid(req->procid, &(acc.gid));
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return (FALSE);
    }

    error_no = proc_get_umask(req->procid, &umask);
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return (FALSE);
    }

    error_no = fs_open_file(pathname,
			 req->param.par_open.oflag,
			 req->param.par_open.mode & (~umask),
			 &acc, startip, &newip);
    if (error_no) {
#ifdef notdef
	printf("open systemcall: Not found entry.\n");
#endif
	/* ファイルがオープンできない */
	put_response(rdvno, error_no, -1, 0);
	return (FALSE);
    }

    if ((newip->i_mode & S_IFMT) == S_IFDIR) {
#ifdef notdef
	W uid, euid;
#endif

	/* ファイルは、ディレクトリだった
	 * エラーとする
	 */

	/* root ユーザの場合には、
	 * 成功でもよい
	 */
#ifdef notdef
	if (proc_get_uid(req->procid, &uid)) {
	    put_response(rdvno, EINVAL, -1, 0);
	    return (FALSE);
	}
	if (proc_get_uid(req->procid, &euid)) {
	    put_response(rdvno, EINVAL, -1, 0);
	    return (FALSE);
	}
	if ((uid != 0) && (euid != 0)) {
	    fs_close_file(newip);
	    put_response(rdvno, EISDIR, -1, 0);
	    return (FALSE);
	}
#else
	if (acc.uid != SU_UID) {
	    fs_close_file(newip);
	    put_response(rdvno, EACCES, -1, 0);
	    return (FALSE);
	}
#endif
	if (req->param.par_open.oflag != O_RDONLY) {
	    fs_close_file(newip);
	    put_response(rdvno, EISDIR, -1, 0);
	    return (FALSE);
	}
    } else if (newip->i_mode & S_IFCHR) {
	/* スペシャルファイルだった */
	/* デバイスに DEV_OPN メッセージを発信 */
	error_no = sfs_open_device(newip->i_dev, &rsize);
	if (rsize >= 0) {
	    newip->i_size = rsize;
	}
	if (error_no != E_OK) {
	    fs_close_file(newip);
	    put_response(rdvno, EACCES, -1, 0);
	    return (FALSE);
	}
    }

    if (proc_set_file(req->procid, fileid,
		      req->param.par_open.oflag, newip)) {
	fs_close_file(newip);
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    put_response(rdvno, EOK, fileid, 0);
    return (TRUE);
}

/* psc_read_f - ファイルからのデータの読み込み
 */
W psc_read_f(RDVNO rdvno, struct posix_request *req)
{
    W error_no;
    struct file *fp;
    W rlength;
    W rest_length;
    W i, len;
    static B buf[MAX_BODY_SIZE];
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    error_no = proc_get_file(req->procid, req->param.par_read.fileid, &fp);
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return (FALSE);
    } else if (fp == 0) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    } else if (fp->f_inode == 0) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    if (fp->f_omode == O_WRONLY) {
	put_response(rdvno, EBADF, -1, 0);
	return (FALSE);
    }

    if (fp->f_flag & F_PIPE) {
	/* パイプの読み書き */
	put_response(rdvno, ENOTSUP, -1, 0);
	return (FALSE);
    }

    if (fp->f_inode->i_mode & S_IFCHR) {
	if ((fp->f_inode->i_dev & BLOCK_DEVICE_MASK)) {
	    /* ブロックデバイスだった */
	    if (fp->f_offset >= fp->f_inode->i_size) {
		error_no = EOK;
		req->param.par_read.length = 0;
	    }
	}

	rest_length = req->param.par_read.length;
	for (i = 0; rest_length > 0;
	    rest_length -= rlength, i += rlength) {
	    /* MAX_BODY_SIZE 毎にバッファに読み込み */
	    len =
		rest_length >
		MAX_BODY_SIZE ? MAX_BODY_SIZE : rest_length;
	    error_no =
		sfs_read_device(fp->f_inode->i_dev, buf,
				    fp->f_offset + i, len, &rlength);
	    if (error_no)
		break;

	    /* 呼び出したプロセスのバッファへの書き込み */
	    error_no = kcall->region_put(req->caller, req->param.par_read.buf + i,
			rlength, buf);
	    if (error_no || (rlength < len)) {
		i += rlength;
		break;
	    }
	}
	if (error_no) {
	    put_response(rdvno, error_no, -1, 0);
	    return (FALSE);
	}

	fp->f_offset += i;
	put_response(rdvno, EOK, i, 0);
	return (TRUE);
    }
#ifdef DEBUG
    printk
	("posix:read: inode = 0x%x, offset = %d, buf = 0x%x, length = %d\n",
	 fp->f_inode, fp->f_offset, req->param.par_read.buf,
	 req->param.par_read.length);
#endif

    if (fp->f_offset >= fp->f_inode->i_size) {
	put_response(rdvno, EOK, 0, 0);
	return (TRUE);
    }
    for (i = 0, rest_length = req->param.par_read.length;
	 rest_length > 0; rest_length -= rlength, i += rlength) {
	/* MAX_BODY_SIZE 毎にファイルに読み込み */
	len = rest_length > MAX_BODY_SIZE ? MAX_BODY_SIZE : rest_length;
	error_no = fs_read_file(fp->f_inode,
			     fp->f_offset + i, buf, len, &rlength);
	if (error_no) {
	    break;
	}

	/* 呼び出したプロセスのバッファへの書き込み */
	error_no = kcall->region_put(req->caller, req->param.par_read.buf + i,
			 rlength, buf);
	if (error_no || (rlength < len)) {
	    i += rlength;
	    break;
	}
    }
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return (FALSE);
    }
    fp->f_offset += i;
    put_response(rdvno, EOK, i, 0);
    return (TRUE);
}

W psc_write_f(RDVNO rdvno, struct posix_request *req)
{
    W error_no;
    struct file *fp;
    W rlength;
    W i, len;
    static B buf[MAX_BODY_SIZE];
    W rest_length;
#ifdef DEBUG
    W j;
#endif
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    error_no = proc_get_file(req->procid, req->param.par_write.fileid, &fp);
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return (FALSE);
    } else if (fp == 0) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    } else if (fp->f_inode == 0) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    if (fp->f_omode == O_RDONLY) {
	put_response(rdvno, EBADF, -1, 0);
	return (FALSE);
    }

    if (fp->f_flag & F_PIPE) {
	/* パイプの読み書き */
	put_response(rdvno, ENOTSUP, -1, 0);
	return (FALSE);
    }
#ifdef debug
    printk
	("posix:write: inode = 0x%x, offset = %d, buf = 0x%x, length = %d\n",
	 fp->f_inode, fp->f_offset, req->param.par_write.buf,
	 req->param.par_read.length);
#endif
    if ((! (fp->f_inode->i_mode & S_IFCHR)) &&
	(fp->f_offset > fp->f_inode->i_size)) {
      /* 通常ファイルで，書き込む場所がファイルの内容が存在しない場所 */
      /* そこまでを 0 で埋める */
      memset(buf, 0, sizeof(buf));
      for (rest_length = fp->f_offset - fp->f_inode->i_size;
	   rest_length > 0; rest_length -= rlength) {
	len = rest_length > MAX_BODY_SIZE ? MAX_BODY_SIZE : rest_length;
	error_no = fs_write_file(fp->f_inode,
			      fp->f_inode->i_size, buf, len, &rlength);
	if (error_no || (rlength < len)) {
	  break;
	}
      }
    }
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return (FALSE);
    }

    for (i = 0, rest_length = req->param.par_write.length;
	 rest_length > 0; rest_length -= rlength, i += rlength) {
#ifdef DEBUG
	printk
	    ("vget_reg (caller = %d, src addr = 0x%x, size = %d, dst = 0x%x\n",
	     req->caller, req->param.par_write.buf + i,
	     rest_length > MAX_BODY_SIZE ? MAX_BODY_SIZE : rest_length,
	     buf);
#endif
	len = rest_length > MAX_BODY_SIZE ? MAX_BODY_SIZE : rest_length;
	error_no =
	    kcall->region_get(req->caller, req->param.par_write.buf + i, len, buf);
	if (error_no)
	    break;
#ifdef DEBUG
	printk("writedata length = %d\n", len);
	for (j = 0;
	     j <
	     ((rest_length > MAX_BODY_SIZE) ? MAX_BODY_SIZE : rest_length);
	     j++) {
	    char print_buffer[2];
#if 0
	    print_buffer[0] = buf[j];
	    print_buffer[1] = '\0';
	    printk("%s", print_buffer);
#else
	    printk("[%x]", buf[j]);
#endif
	}
	printk("\n");
#endif

	error_no = fs_write_file(fp->f_inode,
			      fp->f_offset + i, buf, len, &rlength);
	if (error_no || (rlength < len)) {
	    i += rlength;
	    break;
	}
    }

    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return (FALSE);
    }

    fp->f_offset += i;
    put_response(rdvno, EOK, i, 0);
    return (TRUE);
}