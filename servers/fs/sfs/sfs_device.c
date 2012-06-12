/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/* sfs_device.c - SFS とデバイスドライバのやりとりを管理する。
 *
 *
 *
 */

static char rcsid[] =
    "$Id: sfs_device.c,v 1.18 2000/05/06 03:53:16 naniwa Exp $";

#include "../posix.h"
#include "../../../include/device.h"
#include "sfs_func.h"


#define BLOCK_SIZE	512




/* sfs_write_device - デバイスにデータを送る(そしてストレージに書き込む)
 *
 */
W sfs_write_device(ID device, B * buf, W start, W length, W * rlength)
{
    DDEV_REQ req;		/* 要求パケット */
    DDEV_RES res;		/* 返答パケット */
    ID res_port;
    W errno;
    ID send_port;
    UW dd;
    T_CMBF pk_cmbf = { NULL, TA_TFIFO, 0, sizeof(res) };

    errno = get_device_info(device, &send_port, &dd);
    if (errno) {
	return (errno);
    }

    res_port = acre_mbf(&pk_cmbf);

    if (res_port <= 0) {
	return (EP_NOMEM);
    }

    if (length > MAX_BODY_SIZE) {	/* Kludge!! */
	return (EP_INVAL);
    }

    req.header.mbfid = res_port;
    req.header.msgtyp = DEV_WRI;
    req.body.wri_req.dd = dd;
    req.body.wri_req.start = start;
    req.body.wri_req.size = length;
    memcpy(req.body.wri_req.dt, buf, length);
    errno = snd_mbf(send_port, sizeof(req), &req);
    if (errno != E_OK) {
	dbg_printf("cannot send packet. %d\n", errno);	/* */
	del_mbf(res_port);
	return (EP_NODEV);
    }

    *rlength = sizeof(res);
    errno = rcv_mbf((UB *) & res, (INT *) rlength, res_port);
    if (errno != E_OK) {
	dbg_printf("cannot send packet. %d\n", errno);	/* */
	del_mbf(res_port);
	return (EP_NODEV);
    }

    *rlength = res.body.wri_res.a_size;
    del_mbf(res_port);
    return (res.body.wri_res.errinfo);
}


/* sfs_read_device - デバイスからデータを読み込む
 *
 */
W sfs_read_device(ID device, B * buf, W start, W length, W * rlength)
{
    DDEV_REQ req;		/* 要求パケット */
    DDEV_RES res;		/* 返答パケット */
    ID res_port;
    W errno;
    W rest_length;
    W rsize;
    ID send_port;
    UW dd;
    T_CMBF pk_cmbf = { NULL, TA_TFIFO, 0, sizeof(res) };

    errno = get_device_info(device, &send_port, &dd);
    if (errno) {
	return (errno);
    }

    res_port = acre_mbf(&pk_cmbf);

    if (res_port <= 0) {
	return (EP_NOMEM);
    }
#ifdef notdef
    printk("sfs_read_device: start = %d, size = %d\n", start, length);
#endif				/* FMDEBUG */

    *rlength = 0;
    for (rest_length = length; rest_length > 0; rest_length -= BLOCK_SIZE) {
	req.header.mbfid = res_port;
	req.header.msgtyp = DEV_REA;
	req.body.rea_req.dd = dd;
	req.body.rea_req.start = start + (length - rest_length);
	req.body.rea_req.size
	    = (BLOCK_SIZE > rest_length) ? rest_length : BLOCK_SIZE;
	errno = snd_mbf(send_port, sizeof(req), &req);
	if (errno != E_OK) {
	    dbg_printf("cannot send packet. %d\n", errno);	/* */
	    del_mbf(res_port);
	    return (errno);
	}

	rsize = sizeof(res);
	errno = rcv_mbf((UB *) & res, (VP) & rsize, res_port);
	if (errno != E_OK) {
	    dbg_printf("cannot send packet. %d\n", errno);	/* */
	    del_mbf(res_port);
	    return (errno);
	} else if (res.body.rea_res.errinfo != E_OK) {
	    del_mbf(res_port);
	    return (res.body.rea_res.errinfo);
	}

	memcpy(&buf[length - rest_length], res.body.rea_res.dt,
	      res.body.rea_res.a_size);
	*rlength += res.body.rea_res.a_size;
    }

    del_mbf(res_port);
    return (E_OK);
}

/* sfs_trans_device - デバイスに read メッセージを転送する
 *
 */

W sfs_trans_device(ID device, ID port, ID tskid, B * buf, W start,
		   W length)
{
    DDEV_REQ req;		/* 要求パケット */
    W errno;
    ID send_port;
    UW dd;

    errno = get_device_info(device, &send_port, &dd);
    if (errno) {
	return (errno);
    }

    req.header.mbfid = port;
    req.header.msgtyp = DEV_PRD;
    req.header.tskid = tskid;
    req.body.prd_req.dd = dd;
    req.body.prd_req.buf = buf;
    req.body.prd_req.start = start;
    req.body.prd_req.length = length;
    errno = snd_mbf(send_port, sizeof(req), &req);
    if (errno != E_OK) {
	dbg_printf("cannot send packet. %d\n", errno);	/* */
	return (EP_NODEV);
    }
    return (E_OK);
}

/* sfs_open_device - デバイスに open メッセージを送る
 *
 */

W sfs_open_device(ID device, W * rsize)
{
    DDEV_REQ req;		/* 要求パケット */
    DDEV_RES res;		/* 返答パケット */
    ID res_port;
    W errno;
    ID send_port;
    UW dd;
    INT rlength;
    T_CMBF pk_cmbf = { NULL, TA_TFIFO, 0, sizeof(res) };

    errno = get_device_info(device, &send_port, &dd);
    if (errno) {
	return (errno);
    }

    res_port = acre_mbf(&pk_cmbf);

    if (res_port <= 0) {
	return (EP_NOMEM);
    }

    req.header.mbfid = res_port;
    req.header.msgtyp = DEV_OPN;
    req.body.opn_req.dd = dd;
    errno = snd_mbf(send_port, sizeof(req), &req);
    if (errno != E_OK) {
	dbg_printf("cannot send packet. %d\n", errno);	/* */
	del_mbf(res_port);
	return (EP_NODEV);
    }

    rlength = sizeof(res);
    errno = rcv_mbf((UB *) & res, &rlength, res_port);
    if (errno != E_OK) {
	dbg_printf("cannot send packet. %d\n", errno);	/* */
	del_mbf(res_port);
	return (EP_NODEV);
    }

    *rsize = res.body.opn_res.size;
    del_mbf(res_port);
    return (res.body.opn_res.errinfo);
}

/* sfs_close_device - デバイスに close メッセージを送る
 *
 */

W sfs_close_device(ID device)
{
    DDEV_REQ req;		/* 要求パケット */
    DDEV_RES res;		/* 返答パケット */
    ID res_port;
    W errno;
    ID send_port;
    UW dd;
    INT rlength;
    T_CMBF pk_cmbf = { NULL, TA_TFIFO, 0, sizeof(res) };

    errno = get_device_info(device, &send_port, &dd);
    if (errno) {
	return (errno);
    }

    res_port = acre_mbf(&pk_cmbf);

    if (res_port <= 0) {
	return (EP_NOMEM);
    }

    req.header.mbfid = res_port;
    req.header.msgtyp = DEV_CLS;
    req.body.cls_req.dd = dd;
    errno = snd_mbf(send_port, sizeof(req), &req);
    if (errno != E_OK) {
	dbg_printf("cannot send packet. %d\n", errno);	/* */
	del_mbf(res_port);
	return (EP_NODEV);
    }

    rlength = sizeof(res);
    errno = rcv_mbf((UB *) & res, &rlength, res_port);
    if (errno != E_OK) {
	dbg_printf("cannot send packet. %d\n", errno);	/* */
	del_mbf(res_port);
	return (EP_NODEV);
    }

    del_mbf(res_port);
    return (res.body.opn_res.errinfo);
}
