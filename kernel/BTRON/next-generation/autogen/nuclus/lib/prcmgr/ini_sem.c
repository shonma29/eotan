/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ini_sem
** Description: ���ޥե��κƽ����
*/

ER
ini_sem (W sem_id,W cnt)
{
	return call_btron(BSYS_INI_SEM, sem_id, cnt);
}
