/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rea_smb
** Description: 共有メモリブロックの読み込み
*/

ER
rea_smb (W akey,B *buff,LONG offset,LONG size,LPTR a_size)
{
	return call_btron(BSYS_REA_SMB, akey, *buff, offset, size, a_size);
}
