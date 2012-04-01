/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_dev
** Description: -
*/

ER
gget_dev (W gid,TC *dev)
{
	return call_btron(BSYS_GGET_DEV, gid, *dev);
}
