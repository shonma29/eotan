/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gopn_dev
** Description: -
*/

ER
gopn_dev (TC *dev,B *param)
{
	return call_btron(BSYS_GOPN_DEV, *dev, *param);
}
