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

struct machine_info
{
  unsigned int	rootfs;
  int fstype;
  unsigned int initrd_start;
  unsigned int initrd_size;
};

extern struct machine_info machineInfo;

#endif /*__CORE_BOOT_H__*/
