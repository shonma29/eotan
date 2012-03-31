/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wsta_drg
** Description: -
*/

ER
wsta_drg (W wid,W lock)
{
	return call_btron(BSYS_WSTA_DRG, wid, lock);
}
