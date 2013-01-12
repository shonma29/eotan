/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/**************************************************************************
 *
 *
 */

#ifndef __CORE_BOOT_H__
#define __CORE_BOOT_H__	1

#include "config.h"

#define MODULE_TABLE	(0x00000800)

enum module_type 
{
  kernel	= 0,
  user		= 1,
  lowlib	= 2, 
  server      	= 3,
  driver	= 4
};


/* 複数のモジュールを読み込むためのヘッダ
 *
 *	ヘッダは以下の要素からできている
 *
 *	1) cookie:	マジック番号(ヘッダ識別用)
 *	2) count:	モジュール数
 *	3) module_info:	各モジュールの情報
 *	   length
 *	   vaddr
 *	   entry
 *	   mode
 */
struct module_info
{
  int		length;		/* ディスク上での大きさ */
  int		mem_length;	/* メモリ中での大きさ; 
				   ヘッダを含まず、BSS 領域を含む */
  unsigned int		vaddr;	/* 仮想アドレス	*/
  unsigned int		paddr;	/* 実アドレス	*/
  unsigned int		entry;
  enum module_type	type;
  char		name[MAX_MODULE_NAME];
};

struct machine_info
{
  unsigned int	ext_mem;
  unsigned int	real_mem;
  unsigned int	base_mem;
  unsigned int	rootfs;
};

struct boot_header
{
  unsigned char		cookie;		/* 0001 に固定 */
  int			count;
  struct machine_info	machine;
  struct module_info	modules[0];
};


#endif /*__CORE_BOOT_H__*/
