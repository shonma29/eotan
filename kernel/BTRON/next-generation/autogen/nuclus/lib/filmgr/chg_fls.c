/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_fls
** Description: �ե����륷���ƥ������ѹ�
*/

ER
chg_fls (TC *dev,TC *fs_name,TC *fs_locate)
{
	return call_btron(BSYS_CHG_FLS, *dev, *fs_name, *fs_locate);
}
