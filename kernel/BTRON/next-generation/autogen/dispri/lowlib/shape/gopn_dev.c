/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gopn_dev
** Description: -
*/

ER
bsys_gopn_dev (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gopn_dev *args = (struct bsc_gopn_dev *)argp;
  req.param.par_gopn_dev.*dev = args->*dev;
  req.param.par_gopn_dev.*param = args->*param;

  error = _make_connection(BSC_GOPN_DEV, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
