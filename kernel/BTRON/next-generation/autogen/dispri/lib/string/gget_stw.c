/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_stw
** Description: -
*/

ER
gget_stw (W gid,TC *str,W len,CHGAP *gap,W *pos)
{
	return call_btron(BSYS_GGET_STW, gid, *str, len, *gap, *pos);
}
