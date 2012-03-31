/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: def_msg
** Description: メッセージハンドラの定義
*/

ER
bsys_def_msg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_def_msg *args = (struct bsc_def_msg *)argp;
  req.param.par_def_msg.t_mask = args->t_mask;
  req.param.par_def_msg.msg_hdr = args->msg_hdr;

  error = _make_connection(BSC_DEF_MSG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
