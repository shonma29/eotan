/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: apd_mem
** Description: �����ΰ���ɲ�
*/

ER
apd_mem (LONG phaddr,LONG size)
{
	return call_btron(BSYS_APD_MEM, phaddr, size);
}
