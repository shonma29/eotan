/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_ptr
** Description: -
*/

ER
gget_ptr (PTRSTS *sts,PTRIMAGE *img)
{
	return call_btron(BSYS_GGET_PTR, *sts, *img);
}
