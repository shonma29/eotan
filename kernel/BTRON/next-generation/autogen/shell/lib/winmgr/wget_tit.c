/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_tit
** Description: -
*/

ER
wget_tit (W wid,W *pict,TC *title)
{
	return call_btron(BSYS_WGET_TIT, wid, *pict, *title);
}
