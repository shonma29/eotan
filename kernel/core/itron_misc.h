/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* itron_func.h - 関数の定義
 *
 */

#ifndef __ITRON_FUNC_H__
#define __ITRON_FUNC_H__	1

/* --------------------------------------------------------- */
/* port manager                                              */
/* --------------------------------------------------------- */
extern ER   find_port (B *name, ID *rport);
extern ID   get_port (W minsize, W maxsize);
extern ER   regist_port (B *name, ID port);
extern W    get_req (ID port, VP request, W *size);

extern void     bzero (B *bufp, W size);
extern void     bcopy (UB *buf1, UB *buf2, W size);
extern W        dbg_printf (B *fmt, ...);


#endif /* __ITRON_FUNC_H__ */
