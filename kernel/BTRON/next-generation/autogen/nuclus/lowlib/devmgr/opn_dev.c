/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: opn_dev
** Description: デバイスのオープン
*/

ER
bsys_opn_dev (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_opn_dev *args = (struct bsc_opn_dev *)argp;
  req.param.par_opn_dev.*dev = args->*dev;
  req.param.par_opn_dev.o_mode = args->o_mode;
  req.param.par_opn_dev.*error = args->*error;

  error = _make_connection(BSC_OPN_DEV, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
