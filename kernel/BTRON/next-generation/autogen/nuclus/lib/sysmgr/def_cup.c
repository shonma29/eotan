/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: def_cup
** Description: ���꡼�󥢥åץϥ�ɥ����Ͽ
*/

ER
def_cup (FUNCP clup_hdr)
{
	return call_btron(BSYS_DEF_CUP, clup_hdr);
}
