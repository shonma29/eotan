/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rea_rec
** Description: レコードの読み込み
*/

ER
rea_rec (W fd,W offset,B *buf,LONG size,LPTR r_size,UW *subtype)
{
	return call_btron(BSYS_REA_REC, fd, offset, *buf, size, r_size, *subtype);
}
