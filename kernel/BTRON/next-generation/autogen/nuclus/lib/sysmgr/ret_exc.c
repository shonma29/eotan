/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ret_exc
** Description: 例外処理ハンドラの終了
*/

ER
ret_exc (W ret)
{
	return call_btron(BSYS_RET_EXC, ret);
}
