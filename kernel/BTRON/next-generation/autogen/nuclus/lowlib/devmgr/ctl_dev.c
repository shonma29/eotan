/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ctl_dev
** Description: デバイスの制御
*/

ER
bsys_ctl_dev (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_ctl_dev *args = (struct bsc_ctl_dev *)argp;
  req.param.par_ctl_dev.dd = args->dd;
  req.param.par_ctl_dev.cmd = args->cmd;
  req.param.par_ctl_dev.*param = args->*param;
  req.param.par_ctl_dev.*error = args->*error;

  error = _make_connection(BSC_CTL_DEV, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
