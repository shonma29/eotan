/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_sem
** Description: セマフォの生成
*/

ER
cre_sem (W sem_id,W cnt,UW opt)
{
	return call_btron(BSYS_CRE_SEM, sem_id, cnt, opt);
}
