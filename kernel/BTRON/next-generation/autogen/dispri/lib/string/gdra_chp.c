/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_chp
** Description: -
*/

ER
gdra_chp (W gid,W h,W v,TCODE ch,W mode)
{
	return call_btron(BSYS_GDRA_CHP, gid, h, v, ch, mode);
}
