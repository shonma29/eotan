/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rel_lmb
** Description: ローカルメモリブロックの開放
*/

ER
rel_lmb (B *mptr)
{
	return call_btron(BSYS_REL_LMB, *mptr);
}
