/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_for
** Description: -
*/

ER
gset_for (W gid,RLPTR rlp)
{
	return call_btron(BSYS_GSET_FOR, gid, rlp);
}
