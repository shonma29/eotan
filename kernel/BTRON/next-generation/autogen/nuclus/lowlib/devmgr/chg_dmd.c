/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_dmd
** Description: デバイスのアクセスモード変更
*/

ER
bsys_chg_dmd (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_chg_dmd *args = (struct bsc_chg_dmd *)argp;
  req.param.par_chg_dmd.*dev = args->*dev;
  req.param.par_chg_dmd.mode = args->mode;

  error = _make_connection(BSC_CHG_DMD, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
