/*

  B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

  GNU GENERAL PUBLIC LICENSE
  Version 2, June 1991

  (C) B-Free Project.
  (C) 2001, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/wconsole/wconsole.h,v 1.3 2000/02/27 15:29:26 naniwa Exp $ */

/*
 * $Log: wconsole.h,v $
 * Revision 1.3  2000/02/27 15:29:26  naniwa
 * to work as multi task OS
 *
 * Revision 1.2  2000/02/19 03:00:13  naniwa
 * to support multi window environment
 *
 * Revision 1.1  1999/05/15 09:56:04  naniwa
 * first version
 *
 */

#ifndef __WCONSOLE_H__
#define __WCONSOLE_H__	1

#include "../../kernel/drivers/console/console.h"
#include "../../kernel/drivers/keyboard/keyboard.h"
#include "../../kernel/ITRON/kernlib/device.h"

/*  定数の設定
 */

#define WCONSOLE_DRIVER	"manager.wconsole"

#define WC_NULL		0x00004000
#define WC_WINMOD	0x00004001
#define WC_CHGFCS	0x00004002
#define WC_DEBUG	0x00004003

#define KF_OPEN		0x00000010
#define WD_LOAD		0x00000011
#define WD_ORIG		0x00000012
#define WD_WINSIZ	0x00000013

struct wc_winmod_t
{
  W	mode;
};

union wc_cmd_t
{
  struct wc_winmod_t winmod;
};

struct winsize {
  unsigned short int ws_row;
  unsigned short int ws_col;
  unsigned short int ws_xpixel;
  unsigned short int ws_ypixel;
};
#endif /* __WCONSOLE_H__ */
