/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: del_mpl
** Description: メモリプールの削除
*/

ER
del_mpl (W mpl_id)
{
	return call_btron(BSYS_DEL_MPL, mpl_id);
}
