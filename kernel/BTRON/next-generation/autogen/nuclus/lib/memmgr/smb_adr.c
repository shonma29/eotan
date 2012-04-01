/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: smb_adr
** Description: 共有メモリブロック・アドレスの取り出し
*/

ER
smb_adr (W akey,B **maddr)
{
	return call_btron(BSYS_SMB_ADR, akey, **maddr);
}
