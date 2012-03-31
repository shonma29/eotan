/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ctl_dev
** Description: デバイスの制御
*/

ER
ctl_dev (W dd,UW cmd,B *param,W *error)
{
	return call_btron(BSYS_CTL_DEV, dd, cmd, *param, *error);
}
