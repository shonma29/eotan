/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: def_svc
** Description: �����ƥॳ����ϥ�ɥ�����
*/

ER
def_svc (UW svc_code,FUNCP *svc_hdrs,W opt)
{
	return call_btron(BSYS_DEF_SVC, svc_code, *svc_hdrs, opt);
}
