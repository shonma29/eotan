/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rel_lmb
** Description: ���������֥�å��γ���
*/

ER
rel_lmb (B *mptr)
{
	return call_btron(BSYS_REL_LMB, *mptr);
}
