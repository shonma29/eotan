/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ini_sem
** Description: セマフォの再初期化
*/

ER
ini_sem (W sem_id,W cnt)
{
	return call_btron(BSYS_INI_SEM, sem_id, cnt);
}
