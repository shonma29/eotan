/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lmb_siz
** Description: ローカルメモリブロック・サイズの取り出し
*/

ER
lmb_siz (B *mptr,LPTR size)
{
	return call_btron(BSYS_LMB_SIZ, *mptr, size);
}
