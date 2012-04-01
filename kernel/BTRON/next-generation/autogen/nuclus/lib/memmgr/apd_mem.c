/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: apd_mem
** Description: メモリ領域の追加
*/

ER
apd_mem (LONG phaddr,LONG size)
{
	return call_btron(BSYS_APD_MEM, phaddr, size);
}
