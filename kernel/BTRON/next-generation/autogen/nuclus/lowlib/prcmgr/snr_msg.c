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
bsys_snr_msg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_snr_msg *args = (struct bsc_snr_msg *)argp;
  req.param.par_snr_msg.pid = args->pid;
  req.param.par_snr_msg.*s_msg = args->*s_msg;
  req.param.par_snr_msg.t_mask = args->t_mask;
  req.param.par_snr_msg.*r_msg = args->*r_msg;
  req.param.par_snr_msg.msgsz = args->msgsz;

  error = _make_connection(BSC_SNR_MSG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
