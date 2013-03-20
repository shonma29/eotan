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

#define MODULE_TABLE	(0x00000800)

struct machine_info
{
  unsigned int	ext_mem;
  unsigned int	real_mem;
  unsigned int	base_mem;
  unsigned int	rootfs;
  unsigned int initrd_start;
  unsigned int initrd_size;
};

#endif /*__CORE_BOOT_H__*/
