/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: opn_dev
** Description: デバイスのオープン
*/

ER
opn_dev (TC *dev,UW o_mode,W *error)
{
	return call_btron(BSYS_OPN_DEV, *dev, o_mode, *error);
}
