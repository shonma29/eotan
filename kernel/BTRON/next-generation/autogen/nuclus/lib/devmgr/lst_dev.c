/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lst_dev
** Description: 登録済みデバイスの取り出し
*/

ER
lst_dev (DEV_INFO *dev,W ndev)
{
	return call_btron(BSYS_LST_DEV, *dev, ndev);
}
