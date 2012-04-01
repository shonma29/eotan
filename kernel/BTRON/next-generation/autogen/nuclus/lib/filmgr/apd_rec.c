/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: apd_rec
** Description: レコードの追加
*/

ER
apd_rec (W fd,B *buf,LONG size,W type,UW subtype,UW units)
{
	return call_btron(BSYS_APD_REC, fd, *buf, size, type, subtype, units);
}
