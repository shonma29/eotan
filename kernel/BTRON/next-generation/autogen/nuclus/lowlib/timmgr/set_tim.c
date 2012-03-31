/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_tim
** Description: システム時間の設定
*/

ER
bsys_set_tim (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_set_tim *args = (struct bsc_set_tim *)argp;
  req.param.par_set_tim.time = args->time;
  req.param.par_set_tim.*tz = args->*tz;

  error = _make_connection(BSC_SET_TIM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
