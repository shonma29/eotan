/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_nlk
** Description: リンクの順次取り出し
*/

ER
get_nlk (LINK *lnk)
{
	return call_btron(BSYS_GET_NLK, *lnk);
}
