/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*************************************************************************
 *
 *		2nd BOOT floppy driver routines.
 *
 * $Header: /usr/local/src/master/B-Free/Program/btron-pc/boot/2nd/dma.c,v 1.7 2000/02/27 15:23:51 naniwa Exp $
 *
 * $Log: dma.c,v $
 * Revision 1.7  2000/02/27 15:23:51  naniwa
 * minor change
 *
 * Revision 1.6  1999/12/19 10:53:12  naniwa
 * modified DMA_READ/WRITE parameters
 *
 * Revision 1.5  1999/03/15 01:35:19  monaka
 * Minor modify. Function printf was renamed to boot_printf. Some cast was added. and so on.
 *
 * Revision 1.4  1998/11/20 08:02:25  monaka
 * *** empty log message ***
 *
 * Revision 1.3  1996/07/06 13:08:16  night
 * DMA コントローラの初期化処理を PC 互換機用に変更。
 *
 * Revision 1.2  1996/06/11  16:38:33  night
 * 註釈の変更
 *
 * Revision 1.1  1996/05/11  10:45:01  night
 * 2nd boot (IBM-PC 版 B-FREE OS) のソース。
 * -------------------------------------------------------------------------------
 * Revision 1.2  1995/09/21 15:50:36  night
 * ソースファイルの先頭に Copyright notice 情報を追加。
 *
 * Revision 1.1  1993/10/11  21:28:47  btron
 * btron/386
 *
 * Revision 1.1.1.1  93/01/14  12:30:18  btron
 * BTRON SYSTEM 1.0
 * 
 * Revision 1.1.1.1  93/01/13  16:50:34  btron
 * BTRON SYSTEM 1.0
 * 
 */

#include <itron/types.h>
#include "errno.h"
#include "asm.h"
#include "dma.h"

/****************************************************************************
 *
 */
int
init_dma (void)
{
  return E_OK;
}

/****************************************************************************
 * setup_dma --- ＤＭＡの設定を行う
 *
 */
int
setup_dma (void *addr, int mode, int length, int mask)
{
  UW p;
  
  length -= 1;
  p = (UW)addr;
#ifdef DMADEBUG
  boot_printf ("setup_dma: mode = 0x%x, addr = 0x%x, length = %d, mask = 0x%x\n",
	  mode, addr, length, mask);
#endif
  dis_int();
  outb (DMA_WRITE_SINGLE_MASK, 0x06);

  outb (DMA_CLEAR_BYTE, 0);		/* DMAC のリセット */
  outb (DMA_WRITE_MODE, mode|2);
  outb (DMA_CHANNEL2_ADDR, p & 0xff);
  outb (DMA_CHANNEL2_ADDR, (p >> 8) & 0xff);
  outb (DMA_CHANNEL2_BANK, (p >> 16) & 0x0f);
  outb (DMA_CHANNEL2_COUNT, (length) & 0xff);
  outb (DMA_CHANNEL2_COUNT, ((length) >> 8) & 0xff);
  ena_int();
  outb (DMA_WRITE_SINGLE_MASK, mask);

  return E_OK;
}

