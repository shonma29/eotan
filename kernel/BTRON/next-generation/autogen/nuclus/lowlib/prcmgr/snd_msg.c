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
bsys_snd_msg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_snd_msg *args = (struct bsc_snd_msg *)argp;
  req.param.par_snd_msg.pid = args->pid;
  req.param.par_snd_msg.*msg = args->*msg;
  req.param.par_snd_msg.opt = args->opt;

  error = _make_connection(BSC_SND_MSG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
