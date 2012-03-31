/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_ptr
** Description: メモリポインタの獲得
*/

ER
get_ptr (B *sptr,LONG offset,B **mptr,LONG size)
{
	return call_btron(BSYS_GET_PTR, *sptr, offset, **mptr, size);
}
