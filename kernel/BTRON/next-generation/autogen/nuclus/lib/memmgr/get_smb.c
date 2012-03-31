/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_smb
** Description: 共有メモリブロックの獲得
*/

ER
get_smb (W mpl_id,LONG size,UW opt)
{
	return call_btron(BSYS_GET_SMB, mpl_id, size, opt);
}
