/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_err
** Description: 最新のエラーコードの取り出し
*/

ER
get_err (void)
{
	return call_btron(BSYS_GET_ERR);
}
