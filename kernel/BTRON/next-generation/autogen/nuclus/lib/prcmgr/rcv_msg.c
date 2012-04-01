/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rcv_msg
** Description: メッセージの受信
*/

ER
rcv_msg (W t_mask,MESSAGE *msg,W msgsz,UW opt)
{
	return call_btron(BSYS_RCV_MSG, t_mask, *msg, msgsz, opt);
}
