/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ext_prc
** Description: ���ץ����ν�λ
*/

ER
ext_prc (W exit_code)
{
	return call_btron(BSYS_EXT_PRC, exit_code);
}
