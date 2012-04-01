/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: put_evt
** Description: イベントの発生
*/

ER
put_evt (EVENT *evt,UW opt)
{
	return call_btron(BSYS_PUT_EVT, *evt, opt);
}
