/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: snd_msg
** Description: メッセージの送信
*/

ER
snd_msg (W pid,MESSAGE *msg,UW opt)
{
	return call_btron(BSYS_SND_MSG, pid, *msg, opt);
}
