/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: fls_sts
** Description: ファイルシステムの管理情報取り出し
*/

ER
fls_sts (TC *dev,FS_STATE *buff)
{
	return call_btron(BSYS_FLS_STS, *dev, *buff);
}
