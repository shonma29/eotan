/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ext_prc
** Description: 自プロセスの終了
*/

ER
ext_prc (W exit_code)
{
	return call_btron(BSYS_EXT_PRC, exit_code);
}
