/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ump_mem
** Description: ������֤Υޥåԥ󥰤κ��
*/

ER
ump_mem (B *logaddr)
{
	return call_btron(BSYS_UMP_MEM, *logaddr);
}
