/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: put_evt
** Description: ���٥�Ȥ�ȯ��
*/

ER
put_evt (EVENT *evt,UW opt)
{
	return call_btron(BSYS_PUT_EVT, *evt, opt);
}
