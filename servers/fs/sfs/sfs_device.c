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

#include <string.h>
#include <device.h>
#include <itron/rendezvous.h>
#include "../fs.h"
#include "sfs_func.h"


#define BLOCK_SIZE	512




/* sfs_write_device - デバイスにデータを送る(そしてストレージに書き込む)
 *
 */
W sfs_write_device(ID device, B * buf, W start, W length, W * rlength)
{
    devmsg_t packet;
    W errno;
    ID send_port;
    UW dd;
    ER_UINT rsize;

    errno = get_device_info(device, &send_port, &dd);
    if (errno) {
	return (errno);
    }

    if (length > MAX_BODY_SIZE) {	/* Kludge!! */
	return (EINVAL);
    }

    packet.req.header.msgtyp = DEV_WRI;
    packet.req.body.wri_req.dd = dd;
    packet.req.body.wri_req.start = start;
    packet.req.body.wri_req.size = length;
    memcpy(packet.req.body.wri_req.dt, buf, length);
    rsize = cal_por(send_port, 0xffffffff, &packet, sizeof(packet.req));
    if (rsize < 0) {
	dbg_printf("cal_por error = %d\n", rsize);	/* */
	return (ENODEV);
    }

    *rlength = packet.res.body.wri_res.a_size;
    return (packet.res.body.wri_res.errinfo);
}


/* sfs_read_device - デバイスからデータを読み込む
 *
 */
W sfs_read_device(ID device, B * buf, W start, W length, W * rlength)
{
    devmsg_t packet;
    W errno;
    W rest_length;
    ER_UINT rsize;
    ID send_port;
    UW dd;

    errno = get_device_info(device, &send_port, &dd);
    if (errno) {
	return (errno);
    }

#ifdef notdef
    printk("sfs_read_device: start = %d, size = %d\n", start, length);
#endif				/* FMDEBUG */

    *rlength = 0;
    for (rest_length = length; rest_length > 0; rest_length -= BLOCK_SIZE) {
	packet.req.header.msgtyp = DEV_REA;
	packet.req.body.rea_req.dd = dd;
	packet.req.body.rea_req.start = start + (length - rest_length);
	packet.req.body.rea_req.size
	    = (BLOCK_SIZE > rest_length) ? rest_length : BLOCK_SIZE;
	rsize = cal_por(send_port, 0xffffffff, &packet, sizeof(packet.req));
	if (rsize < 0) {
	    dbg_printf("cal_por error = %d\n", rsize);	/* */
	    return (errno);
	}

	if (packet.res.body.rea_res.errinfo != E_OK) {
	    dbg_printf("[FS] sfs_read_device errinfo = %d\n",
		    packet.res.body.rea_res.errinfo);
	    return (packet.res.body.rea_res.errinfo);
	}

	memcpy(&buf[length - rest_length], packet.res.body.rea_res.dt,
	      packet.res.body.rea_res.a_size);
	*rlength += packet.res.body.rea_res.a_size;
    }

    return (E_OK);
}

/* sfs_open_device - デバイスに open メッセージを送る
 *
 */

W sfs_open_device(ID device, W * rsize)
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

    packet.req.header.msgtyp = DEV_OPN;
    packet.req.body.opn_req.dd = dd;
    rlength = cal_por(send_port, 0xffffffff, &packet, sizeof(packet.req));
    if (rlength < 0) {
	dbg_printf("cal_por error = %d\n", rsize);	/* */
	return (ENODEV);
    }

    *rsize = packet.res.body.opn_res.size;
    return (packet.res.body.opn_res.errinfo);
}

/* sfs_close_device - デバイスに close メッセージを送る
 *
 */

W sfs_close_device(ID device)
{
    devmsg_t packet;
    W errno;
    ID send_port;
    UW dd;
    ER_UINT rsize;

    errno = get_device_info(device, &send_port, &dd);
    if (errno) {
	return (errno);
    }

    packet.req.header.msgtyp = DEV_CLS;
    packet.req.body.cls_req.dd = dd;
    rsize = cal_por(send_port, 0xffffffff, &packet, sizeof(packet.req));
    if (rsize < 0) {
	dbg_printf("cal_por error = %d\n", rsize);	/* */
	return (ENODEV);
    }

    return (packet.res.body.opn_res.errinfo);
}
