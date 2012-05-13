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
#include "../../kernel/lib/libkernel/device.h"

/*  定数の設定
 */

#define WCONSOLE_DRIVER	"manager.wconsole"

#endif /* __WCONSOLE_H__ */
