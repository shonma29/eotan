/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_bmp
** Description: -
*/

ER
gget_bmp (W gid,BMPTR bmap)
{
	return call_btron(BSYS_GGET_BMP, gid, bmap);
}
