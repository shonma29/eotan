/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gen_fil
** Description: �ե������ľ������
*/

ER
gen_fil (LINK *lnk,F_LINK *ref,W opt)
{
	return call_btron(BSYS_GEN_FIL, *lnk, *ref, opt);
}
