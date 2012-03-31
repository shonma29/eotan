/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_evt
** Description: -
*/

ER
wget_evt (WEVENT *evt,W mode)
{
	return call_btron(BSYS_WGET_EVT, *evt, mode);
}
