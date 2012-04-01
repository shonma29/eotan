/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: loc_rec
** Description: レコードのロック
*/

ER
loc_rec (W fd,W mode)
{
	return call_btron(BSYS_LOC_REC, fd, mode);
}
