/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: smb_key
** Description: 共有メモリブロック・アクセスキーの取り出し
*/

ER
smb_key (B *maddr)
{
	return call_btron(BSYS_SMB_KEY, *maddr);
}
