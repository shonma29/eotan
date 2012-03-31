/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: dev_sts
** Description: デバイスの管理情報の取り出し
*/

ER
dev_sts (TC *dev,DEV_STATE *buf)
{
	return call_btron(BSYS_DEV_STS, *dev, *buf);
}
