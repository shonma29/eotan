/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_krp
** Description: キーリピート間隔の取り出し
*/

ER
get_krp (W *offset,W *interval)
{
	return call_btron(BSYS_GET_KRP, *offset, *interval);
}
