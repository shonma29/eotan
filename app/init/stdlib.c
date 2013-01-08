/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/

/* $Id: stdlib.c,v 1.5 1999/05/10 02:57:17 naniwa Exp $ */

/*
 * $Log: stdlib.c,v $
 * Revision 1.5  1999/05/10 02:57:17  naniwa
 * add fprintf() and modified to user vfprintf()
 *
 * Revision 1.4  1998/12/08 05:38:39  monaka
 * I init has been included the Mitten Scripting Language since this release.
 * But, it's just a first preview. So you can't use many convenience
 * features. Mitten works extension scripting environment like Tcl, Guile,
 * Windows Scripting Host, and so on. If you wished, you may connect your
 * function written in C to Mitten operator. If you wished, you may call
 * Mitten from C.
 *
 * Revision 1.3  1997/07/06 11:50:02  night
 * デバッグ文の引数指定がまちがっていたので、修正した。
 *
 * Revision 1.2  1997/04/24 15:38:06  night
 * printf() 関数の処理を kernlib の dbg_printf() と同様のものにした。
 *
 * Revision 1.1  1996/07/25  16:03:44  night
 * IBM PC 版への最初の登録
 *
 *
 */
#include <device.h>
#include <stdarg.h>
#include <string.h>
#include <itron/rendezvous.h>
#include "../../lib/libserv/libserv.h"
#include "init.h"

static W putc(W ch, FILE * port);
static void _putc(const B ch);
static W __putc(W ch, FILE * port);
static void fflush(FILE * port);
static W writechar(ID port, UB * buf, W length);
static W readchar(ID port);


/*
 *
 */
W printf(B *fmt, ...)
{
    W len;
    va_list ap;

    va_start(ap, fmt);
    len = vnprintf(_putc, fmt, ap);
    fflush(stdout);
    return len;
}

static W putc(W ch, FILE * port)
{
    port->buf[port->count] = ch;
    port->count++;

    if (ch == '\n') {
	writechar(port->device, port->buf, port->count);
	port->count = 0;
    } else if (port->count >= port->bufsize) {
	writechar(port->device, port->buf, port->count);
	port->count = 0;
    }

    return (ch);
}

static void _putc(const B ch) {
	putc(ch, stdout);
}

static W __putc(W ch, FILE * port)
{
    UB buf[1];

    buf[0] = ch;
    writechar(port->device, buf, 1);
    return (ch);
}

static void fflush(FILE * port)
{
    writechar(port->device, port->buf, port->count);
    port->count = 0;
}

static W writechar(ID port, UB * buf, W length)
{
    devmsg_t packet;
    ER_UINT rsize;

    packet.req.header.msgtyp = DEV_WRI;
    packet.req.body.wri_req.dd = 0xAA;
    packet.req.body.wri_req.size = length;
    memcpy(packet.req.body.wri_req.dt, buf, length);
    rsize = cal_por(port, 0xffffffff, &packet, sizeof(packet.req));
    if (rsize < 0) {
	dbg_printf("[INIT] writechar cal_port[%d] error = %d\n",
		port, rsize);
	return (-1);
    }
    if (packet.res.body.wri_res.errcd != E_OK) {
	dbg_printf("[INIT] writechar[%d] res.errcd = %d\n",
		port, packet.res.body.wri_res.errcd);
	return (0);
    }
    return (1);
}

W isprint(W ch)
{
    if ((ch < 0x20) || (ch >= 0x7f)) {
	return (FALSE);
    }
    return (TRUE);
}

W isspace(W ch)
{
    if ((ch == ' ') || (ch == '\t') || (ch == '\n'))
	return (TRUE);
    return (FALSE);
}

W getc(FILE * port)
{
    W ch;

    ch = readchar(port->device);
    if (ch != '\b')
	__putc(ch, stdout);
    return (ch);
}


static W readchar(ID port)
{
    devmsg_t packet;
    ER_UINT rsize;

    packet.req.header.msgtyp = DEV_REA;
    packet.req.body.rea_req.dd = 0xAA;
    packet.req.body.rea_req.size = 1;
    rsize = cal_por(port, 0xffffffff, &packet, sizeof(packet.req));
    if (rsize < 0) {
	dbg_printf("[INIT] readchar cal_por[%d] error = %d\n", port, rsize);
	return (-1);
    }
    if (packet.res.body.rea_res.dt[0] != 0) {
	return (packet.res.body.rea_res.dt[0]);
    }
    return (0);
}
