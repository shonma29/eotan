/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wset_tit
** Description: -
*/

ER
wset_tit (W wid,W pict,TC *title,W mode)
{
	return call_btron(BSYS_WSET_TIT, wid, pict, *title, mode);
}
