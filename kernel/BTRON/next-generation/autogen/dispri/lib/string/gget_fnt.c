/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_fnt
** Description: -
*/

ER
gget_fnt (W gid,FONTSPEC *fnt,FONTINF *inf)
{
	return call_btron(BSYS_GGET_FNT, gid, *fnt, *inf);
}
