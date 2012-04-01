/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: map_mem
** Description: メモリ空間のマッピング
*/

ER
map_mem (LONG phaddr,B **logaddr,LONG size,UW opt)
{
	return call_btron(BSYS_MAP_MEM, phaddr, **logaddr, size, opt);
}
