/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ret_msg
** Description: メッセージハンドラの終了
*/

ER
ret_msg (W ret)
{
	return call_btron(BSYS_RET_MSG, ret);
}
