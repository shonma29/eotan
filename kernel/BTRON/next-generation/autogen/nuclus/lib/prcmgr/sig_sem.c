/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: sig_sem
** Description: ���ޥե��γ���
*/

ER
sig_sem (W sem_id,W cnt)
{
	return call_btron(BSYS_SIG_SEM, sem_id, cnt);
}
