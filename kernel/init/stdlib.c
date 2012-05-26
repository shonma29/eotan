/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/

/* $Id: stdlib.c,v 1.5 1999/05/10 02:57:17 naniwa Exp $ */
static char rcsid[] =
    "$Id: stdlib.c,v 1.5 1999/05/10 02:57:17 naniwa Exp $";

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
#include "../lib/libkernel/libkernel.h"
#include "init.h"

static W putc(W ch, FILE * port);
static void _putc(const B ch);
static W __putc(W ch, FILE * port);
static void fflush(FILE * port);
static W writechar(ID port, ID resport, UB * buf, W length);
static W readchar(ID port, ID resport);


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
	writechar(port->device, dev_recv, port->buf, port->count);
	port->count = 0;
    } else if (port->count >= port->bufsize) {
	writechar(port->device, dev_recv, port->buf, port->count);
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
    writechar(port->device, dev_recv, buf, 1);
    return (ch);
}

static void fflush(FILE * port)
{
    writechar(port->device, dev_recv, port->buf, port->count);
    port->count = 0;
}

static W writechar(ID port, ID resport, UB * buf, W length)
{
    DDEV_REQ req;		/* 要求パケット */
    DDEV_RES res;		/* 返答パケット */
    W rsize;
    ER error;

    req.header.mbfid = resport;
    req.header.msgtyp = DEV_WRI;
    req.body.wri_req.dd = 0xAA;
    req.body.wri_req.size = length;
    memcpy(req.body.wri_req.dt, buf, length);
    error = snd_mbf(port, sizeof(req), &req);
    if (error != E_OK) {
	dbg_printf("cannot send packet. %d(%s, %d)\n", error, __FILE__,
		   __LINE__);
	return (-1);
    }
    rsize = sizeof(res);
    error = rcv_mbf(&res, (INT *) & rsize, resport);
    if (res.body.wri_res.errcd != E_OK) {
	dbg_printf("%d\n", res.body.wri_res.errcd);
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

    ch = readchar(port->device, dev_recv);
    if (ch != '\b')
	__putc(ch, stdout);
    return (ch);
}


static W readchar(ID port, ID resport)
{
    DDEV_REQ req;		/* 要求パケット */
    DDEV_RES res;		/* 返答パケット */
    W rsize;
    ER error;

    req.header.mbfid = resport;
    req.header.msgtyp = DEV_REA;
    req.body.rea_req.dd = 0xAA;
    req.body.rea_req.size = 1;
    error = snd_mbf(port, sizeof(req), &req);
    if (error != E_OK) {
	printf("cannot send packet. %d\n", error);
	return (-1);
    }
    rsize = sizeof(res);
    rcv_mbf(&res, (INT *) & rsize, resport);
    if (res.body.rea_res.dt[0] != 0) {
	return (res.body.rea_res.dt[0]);
    }
    return (0);
}
