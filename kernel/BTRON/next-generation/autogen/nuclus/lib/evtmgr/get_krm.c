/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_krm
** Description: 自動リピート対象キーの取り出し
*/

ER
get_krm (KEPMAP keymap)
{
	return call_btron(BSYS_GET_KRM, keymap);
}
