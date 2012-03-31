/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_tim
** Description: システム時間の取り出し
*/

ER
bsys_get_tim (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_tim *args = (struct bsc_get_tim *)argp;
  req.param.par_get_tim.time = args->time;
  req.param.par_get_tim.*tz = args->*tz;

  error = _make_connection(BSC_GET_TIM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
