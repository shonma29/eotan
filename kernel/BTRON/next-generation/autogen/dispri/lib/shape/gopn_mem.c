/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gopn_mem
** Description: -
*/

ER
gopn_mem (TC *dev,BMPTR bmap,B *param)
{
	return call_btron(BSYS_GOPN_MEM, *dev, bmap, *param);
}
