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
bsys_rcv_msg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_rcv_msg *args = (struct bsc_rcv_msg *)argp;
  req.param.par_rcv_msg.t_mask = args->t_mask;
  req.param.par_rcv_msg.*msg = args->*msg;
  req.param.par_rcv_msg.msgsz = args->msgsz;
  req.param.par_rcv_msg.opt = args->opt;

  error = _make_connection(BSC_RCV_MSG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
