/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_stp
** Description: -
*/

ER
gdra_stp (W gid,W h,W v,TC *str,W len,W mode)
{
	return call_btron(BSYS_GDRA_STP, gid, h, v, *str, len, mode);
}
