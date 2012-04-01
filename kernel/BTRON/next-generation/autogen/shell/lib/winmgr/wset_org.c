/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wset_org
** Description: -
*/

ER
wset_org (W wid,W parent,RPTR org)
{
	return call_btron(BSYS_WSET_ORG, wid, parent, org);
}
