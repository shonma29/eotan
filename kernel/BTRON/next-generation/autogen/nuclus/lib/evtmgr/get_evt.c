/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_evt
** Description: イベントの取り出し
*/

ER
get_evt (W t_mask,EVENT *evt,UW opt)
{
	return call_btron(BSYS_GET_EVT, t_mask, *evt, opt);
}
