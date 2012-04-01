/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gcom_bmp
** Description: -
*/

ER
gcom_bmp (W gid,RPTR dr,C_BMPTR cbmp,LPTR stat)
{
	return call_btron(BSYS_GCOM_BMP, gid, dr, cbmp, stat);
}
