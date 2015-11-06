/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/* device.c - FS とデバイスドライバのやりとりを管理する。
 *
 *
 *
 */

#include <string.h>
#include <device.h>
#include <core.h>
#include <nerve/kcall.h>
#include "fs.h"



/* write_device - デバイスにデータを送る(そしてストレージに書き込む)
 *
 */
W write_device(ID device, B * buf, W start, W length, W * rlength)
{
    devmsg_t packet;
    devmsg_t *p = &packet;
    W error_no;
    ID send_port;
    UW dd;
    ER_UINT rsize;

    error_no = get_device_info(device, &send_port, &dd);
    if (error_no) {
	return (error_no);
    }

    if (length > MAX_BODY_SIZE) {	/* Kludge!! */
	return (EINVAL);
    }

    packet.Rwrite.operation = operation_write;
    packet.Rwrite.channel = dd;
    packet.Rwrite.offset = start;
    packet.Rwrite.length = length;
    packet.Rwrite.data = (UB*)buf;
    rsize = kcall->port_call(send_port, &p, sizeof(p));
    if (rsize < 0) {
	dbg_printf("fs: cal_por error = %d\n", rsize);	/* */
	return (ENODEV);
    }

    *rlength = (packet.Twrite.length > 0)? packet.Twrite.length:0;
    return (packet.Twrite.length == length)? E_OK:E_SYS;
}


/* read_device - デバイスからデータを読み込む
 *
 */
W read_device(ID device, B * buf, W start, W length, W * rlength)
{
    devmsg_t packet;
    devmsg_t *p = &packet;
    W error_no;
    W rest_length;
    ER_UINT rsize;
    ID send_port;
    UW dd;

    error_no = get_device_info(device, &send_port, &dd);
    if (error_no) {
	return (error_no);
    }

    *rlength = 0;
    for (rest_length = length; rest_length > 0;) {
	packet.Rread.operation = operation_read;
	packet.Rread.channel = dd;
	packet.Rread.offset = start + (length - rest_length);
	packet.Rread.length
	    = (DEV_BUF_SIZE > rest_length) ?
		    rest_length : DEV_BUF_SIZE;
	packet.Rread.data = (UB*)(&buf[length - rest_length]);
	rsize = kcall->port_call(send_port, &p, sizeof(p));
	if (rsize < 0) {
	    dbg_printf("fs: cal_por error = %d\n", rsize);	/* */
	    return (error_no);
	}

	if (packet.Tread.length == 0) {
	    break;
        }

	if (packet.Tread.length < 0) {
	    dbg_printf("fs: read_device errinfo = %d\n",
		    packet.Tread.length);
	    return (packet.Tread.length);
	}

	*rlength += packet.Tread.length;
	rest_length -= packet.Tread.length;
    }

    return (E_OK);
}
