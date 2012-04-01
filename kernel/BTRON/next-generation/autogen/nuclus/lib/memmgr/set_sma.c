/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_sma
** Description: 共有メモリブロック属性の設定
*/

ER
set_sma (W akey,UW attr)
{
	return call_btron(BSYS_SET_SMA, akey, attr);
}
