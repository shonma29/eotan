/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wwai_rsp
** Description: -
*/

ER
wwai_rsp (WEVENT *evt,W cmd,UW tmout)
{
	return call_btron(BSYS_WWAI_RSP, *evt, cmd, tmout);
}
