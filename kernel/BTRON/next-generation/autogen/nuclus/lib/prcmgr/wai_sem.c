/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wai_sem
** Description: ���ޥե��γ���
*/

ER
wai_sem (W sem_id)
{
	return call_btron(BSYS_WAI_SEM, sem_id);
}
