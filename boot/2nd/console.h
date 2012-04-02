/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Source: /usr/local/src/master/B-Free/Program/btron-pc/boot/2nd/console.h,v $
 *
 * $Date: 1999/03/15 01:35:17 $
 * $Author: monaka $
 *
 *
 * description:
 *
 */

#ifndef __CONSOLE_H__
#define __CONSOLE_H__	1

#define MAX_HEIGHT	24
#define MAX_WIDTH	80

extern void console_clear (void);
extern void write_cr (void);
extern void write_tab (void);
extern int init_console (void);
extern int putchar (int ch);


#endif /* __CONSOLE_H__ */


