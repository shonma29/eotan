/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/NETWORK/etcpip/tcpip.h,v 1.1 1998/11/24 15:28:31 monaka Exp $ */


/* 
 * $Log: tcpip.h,v $
 * Revision 1.1  1998/11/24 15:28:31  monaka
 * Some files has copied from NETWORK/tcpip and modified.
 *
 * Revision 1.2  1997/05/08 15:10:06  night
 * ファイル内の文字コードを SJIS から EUC に変更した。
 *
 * Revision 1.1  1997/05/06 12:46:23  night
 * 最初の登録
 *
 *
 */

#ifndef __TCPIP_H__
#define __TCPIP_H__	1

#include "../../ITRON/h/itron.h"
#include "../../ITRON/h/errno.h"
#include "../../BTRON/device/console/console.h"
#include "../../BTRON/servers/port-manager.h"


#include "ether.h"
#include "arp.h"
#include "ip.h"
#include "udp.h"
#include "tcp.h"
#include "global.h"


#endif /* __TCPIP_H__ */


