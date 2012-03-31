/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/NETWORK/tcpip/lowlib.c,v 1.2 1997/05/08 15:10:06 night Exp $ */
static char rcs[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/NETWORK/tcpip/lowlib.c,v 1.2 1997/05/08 15:10:06 night Exp $";


/* 
 * $Log: lowlib.c,v $
 * Revision 1.2  1997/05/08 15:10:06  night
 * ファイル内の文字コードを SJIS から EUC に変更した。
 *
 * Revision 1.1  1997/05/06 12:46:23  night
 * 最初の登録
 *
 *
 */

#include "tcpip.h"



/* TCP/IP マネージャへアクセスするための LOWLIB 層
 *
 */

/* Lowlib を特定のタスクに接続する。
 */
ER
attach ()
{
  
}

/* Lowlib への要求をソフトウェア割り込みにて受けとる。
 */
ER
entry ()
{

}
