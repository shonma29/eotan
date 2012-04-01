/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lmb_sts
** Description: ローカルメモリ領域の状態取り出し
*/

ER
lmb_sts (LM_STATE *buff)
{
	return call_btron(BSYS_LMB_STS, *buff);
}
