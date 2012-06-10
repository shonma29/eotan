/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002-2003, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/console/console_internal.h,v 1.5 2000/07/09 16:44:46 kishida0 Exp $ */

/*
 * $Log: console_internal.h,v $
 * Revision 1.5  2000/07/09 16:44:46  kishida0
 * fix some warning (sample for other devices)
 *
 * Revision 1.4  2000/02/27 15:25:17  naniwa
 * minor fix
 *
 * Revision 1.3  1999/04/18 18:43:20  monaka
 * Fix for moving Port-manager and libkernel.a  to ITRON. I guess it is reasonable. At least they should not be in BTRON/.
 *
 * Revision 1.2  1998/11/30 13:16:56  night
 * マクロ MAX_PATTERN_SIZE の定義。
 * このマクロは、コンソールデバイスドライバにバックグラウンドパターンの描
 * 画や region の描画などを行うときの一時バッファ (copy_buffer) のサイズ
 * を指定するために使用する。
 * とりあえず、10KB (1024 * 10) 指定している。
 *
 * Revision 1.1  1997/08/31 13:59:23  night
 * 最初の登録
 *
 *
 */


#ifndef __CGA_H__
#define __CGA_H__	1

#include "../../include/itron/types.h"

#define MAX_COLUMN	80
#define MAX_LINE	25

#define TEXT_VRAM_ADDR	0x800B8000	/* TEXT VRAM のアドレス */
#define TEXT_VRAM_SIZE	2000

#define GDC_ADDR	0x03d4
#define GDC_DATA	0x03d5

#define GDC_STAT	0x03d4
#define GDC_COMMAND	0x03d5

typedef enum { NORMAL = 1, REVERSE = 7} TEXTATTR;

#define VGA_TEXT_BLACK	0
#define VGA_TEXT_WHITE	7
#define VGA_TEXT_ATTR(fg, bg) ((bg << 4) |  fg)

extern void set_curpos (W x, W y);
extern void move_curpos (W x, W y);
extern ER write_char (UB ch);
extern void clear_console (void);
extern void clear_rest_line(void);
extern void clear_rest_screen(void);

#endif /* __CGA_H__ */
