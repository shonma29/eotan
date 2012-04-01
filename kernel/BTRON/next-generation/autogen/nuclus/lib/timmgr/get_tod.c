/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_tod
** Description: 日付時刻の設定/変換
*/

ER
get_tod (DATE_TIM *date_tim,LONG time,W local)
{
	return call_btron(BSYS_GET_TOD, *date_tim, time, local);
}
