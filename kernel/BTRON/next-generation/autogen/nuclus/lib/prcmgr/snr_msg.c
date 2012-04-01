/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: snr_msg
** Description: メッセージの送受信
*/

ER
snr_msg (W pid,MESSAGE *s_msg,W t_mask,MESSAGE *r_msg,W msgsz)
{
	return call_btron(BSYS_SNR_MSG, pid, *s_msg, t_mask, *r_msg, msgsz);
}
