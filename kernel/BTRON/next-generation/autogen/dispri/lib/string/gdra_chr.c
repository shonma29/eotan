/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_chr
** Description: -
*/

ER
gdra_chr (W gid,TCODE ch,W mode)
{
	return call_btron(BSYS_GDRA_CHR, gid, ch, mode);
}
