/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/pcmcia/funcs.h,v 1.1 1999/04/21 17:56:28 kishida0 Exp $ */

/* 
 * $Log: funcs.h,v $
 * Revision 1.1  1999/04/21 17:56:28  kishida0
 * for debug
 *
 * Revision 1.1  1998/12/19 07:50:25  monaka
 * Pre release version.
 *
 */

/*
 *
 */
#ifndef	__FUNCS_H__
#define	__FUNCS_H__	1

/* pcmcia.c */
extern ER	process_request (DDEV_REQ *req);
extern ER	open_pcmcia (ID caller, DDEV_OPN_REQ *packet);
extern ER	close_pcmcia (ID caller, DDEV_CLS_REQ *packet);
extern ER	read_pcmcia (ID caller, DDEV_REA_REQ *packet);
extern ER	write_pcmcia (ID caller, DDEV_WRI_REQ *packet);
extern ER	control_pcmcia (ID caller, DDEV_CTL_REQ *packet);


/* pcmcialow.c */
extern ER	probe (struct device *dev);
/*** ここに **/

/* misc.c */
extern void	init_log (void);
extern void	print_digit (UW d, UW base);
extern W	printf (B *fmt,...);
extern W	put_string (B *line, ID port);
extern W 	putc (int ch, ID port);



#endif /* __FUNCS_H__ */
