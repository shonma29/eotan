/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ofl_sts
** Description: ファイル情報の取り出し
*/

ER
ofl_sts (W fd,TC *name,F_STATE *stat,F_LOCATE *locat)
{
	return call_btron(BSYS_OFL_STS, fd, *name, *stat, *locat);
}
