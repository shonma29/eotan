/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ump_mem
** Description: メモリ空間のマッピングの削除
*/

ER
ump_mem (B *logaddr)
{
	return call_btron(BSYS_UMP_MEM, *logaddr);
}
