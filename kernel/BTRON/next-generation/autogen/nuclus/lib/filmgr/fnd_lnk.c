/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: fnd_lnk
** Description: リンクレコードの検索
*/

ER
fnd_lnk (W fd,W mode,LINK *lnk,UW subtype,LPTR recnum)
{
	return call_btron(BSYS_FND_LNK, fd, mode, *lnk, subtype, recnum);
}
