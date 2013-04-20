/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* misc.h ---
 *
 *
 */

#ifndef __CORE_MISC_H__
#define __CORE_MISC_H__	1

/* 2 の冪乗でしか使えないが，効率は良い */
#define ROUNDUP(x,align)	((((int)x) + (align) - 1) & ~((align) - 1))
#define CUTDOWN(x,align)	((((int)x) & ~((align) - 1)))

#endif /* __CORE_MISC_H__ */
