/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wai_sem
** Description: セマフォの獲得
*/

ER
wai_sem (W sem_id)
{
	return call_btron(BSYS_WAI_SEM, sem_id);
}
