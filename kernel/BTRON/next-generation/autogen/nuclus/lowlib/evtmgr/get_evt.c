/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_evt
** Description: イベントの取り出し
*/

ER
bsys_get_evt (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_evt *args = (struct bsc_get_evt *)argp;
  req.param.par_get_evt.t_mask = args->t_mask;
  req.param.par_get_evt.*evt = args->*evt;
  req.param.par_get_evt.opt = args->opt;

  error = _make_connection(BSC_GET_EVT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
