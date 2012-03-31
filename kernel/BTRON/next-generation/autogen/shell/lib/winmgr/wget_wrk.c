/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_wrk
** Description: -
*/

ER
wget_wrk (W wid,RPTR r)
{
	return call_btron(BSYS_WGET_WRK, wid, r);
}
