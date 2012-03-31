/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wwai_rsp
** Description: -
*/

ER
bsys_wwai_rsp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wwai_rsp *args = (struct bsc_wwai_rsp *)argp;
  req.param.par_wwai_rsp.*evt = args->*evt;
  req.param.par_wwai_rsp.cmd = args->cmd;
  req.param.par_wwai_rsp.tmout = args->tmout;

  error = _make_connection(BSC_WWAI_RSP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
