/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rsz_lmb
** Description: ローカルメモリブロックのサイズ変更
*/

ER
rsz_lmb (B **n_mptr,B *mptr,LONG size,UW opt)
{
	return call_btron(BSYS_RSZ_LMB, **n_mptr, *mptr, size, opt);
}
