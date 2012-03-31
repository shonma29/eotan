/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rgt_smb
** Description: 共有メモリブロックの再獲得
*/

ER
rgt_smb (W akey)
{
	return call_btron(BSYS_RGT_SMB, akey);
}
