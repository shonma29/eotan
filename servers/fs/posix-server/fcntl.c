/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/*
 * $Log: fcntl.c,v $
 * Revision 1.4  2000/02/20 09:40:04  naniwa
 * minor fix
 *
 * Revision 1.3  2000/01/30 03:04:48  naniwa
 * minor fix
 *
 * Revision 1.2  2000/01/29 16:26:53  naniwa
 * partially implemented
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */
#include <device.h>
#include "posix.h"

/* ctl_device - デバイスにコントロールメッセージを送る
 *
 */
static W control_device(ID device, struct posix_request *preq)
{
    devmsg_t packet;
    W errno;
    ID send_port;
    UW dd;
    ER_UINT rlength;

    errno = get_device_info(device, &send_port, &dd);
    if (errno) {
	return (errno);
    }

    packet.req.header.msgtyp = DEV_CTL;
    packet.req.header.tskid = preq->caller;
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
	errno = vget_reg(preq->caller, preq->param.par_fcntl.arg,
			 packet.req.body.ctl_req.len,
			 packet.req.body.ctl_req.param);
	if (errno) {
	    dbg_printf("fctl: vget_reg error\n");
	    return (errno);
	}
    }

    rlength = cal_por(send_port, 0xffffffff, &packet, sizeof(packet.req));
    if (rlength < 0) {
	dbg_printf("cannot call port. %d\n", rlength);	/* */
	return (EP_NODEV);
    }

    return (packet.res.body.ctl_res.errinfo);
}

/* psc_fcntl_f - ファイルに対して特殊な操作を行う。
 */
W psc_fcntl_f(RDVNO rdvno, struct posix_request * req)
{
    W errno;
    struct file *fp;
    ID device;

    errno = proc_get_file(req->procid, req->param.par_fcntl.fileid, &fp);
    if (errno) {
	put_response(rdvno, req, errno, -1, 0, 0);
	return (FAIL);
    } else if (fp == 0) {
	put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	return (FAIL);
    } else if (fp->f_inode == 0) {
	put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	return (FAIL);
    }

    if (fp->f_inode->i_mode & FS_FMT_DEV) {
	/* スペシャルファイルだった
	 */
	device = fp->f_inode->i_dev;

	/* send message to the device.
	 */
	errno = control_device(device, req);
	if (errno) {
	    put_response(rdvno, req, errno, errno, 0, 0);
	    return (FAIL);
	} else {
	    put_response(rdvno, req, EP_OK, errno, 0, 0);
	    return (SUCCESS);
	}
    } else {
	/* とりあえず、サポートしていないというエラーで返す
	 */
	put_response(rdvno, req, EP_NOSUP, 0, 0, 0);
    }

    return (FAIL);
}
