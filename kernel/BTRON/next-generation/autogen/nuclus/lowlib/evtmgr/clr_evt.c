/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: clr_evt
** Description: イベントの削除
*/

ER
bsys_clr_evt (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_clr_evt *args = (struct bsc_clr_evt *)argp;
  req.param.par_clr_evt.t_mask = args->t_mask;
  req.param.par_clr_evt.lask_mask = args->lask_mask;

  error = _make_connection(BSC_CLR_EVT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
