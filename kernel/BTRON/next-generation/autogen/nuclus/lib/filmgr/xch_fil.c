/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: xch_fil
** Description: ファイルの内容交換
*/

ER
xch_fil (W fd_1,W fd_2)
{
	return call_btron(BSYS_XCH_FIL, fd_1, fd_2);
}
