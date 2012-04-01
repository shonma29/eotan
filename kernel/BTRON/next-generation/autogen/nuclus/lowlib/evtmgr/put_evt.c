/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: put_evt
** Description: イベントの発生
*/

ER
bsys_put_evt (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_put_evt *args = (struct bsc_put_evt *)argp;
  req.param.par_put_evt.*evt = args->*evt;
  req.param.par_put_evt.opt = args->opt;

  error = _make_connection(BSC_PUT_EVT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
