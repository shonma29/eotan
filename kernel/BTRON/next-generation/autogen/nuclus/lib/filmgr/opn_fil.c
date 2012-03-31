/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: opn_fil
** Description: ファイルのオープン
*/

ER
opn_fil (LINK *lnk,UW o_mode,TC *pwd)
{
	return call_btron(BSYS_OPN_FIL, *lnk, o_mode, *pwd);
}
