/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_fmd
** Description: ファイルのアクセスモード変更
*/

ER
bsys_chg_fmd (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_chg_fmd *args = (struct bsc_chg_fmd *)argp;
  req.param.par_chg_fmd.*lnk = args->*lnk;
  req.param.par_chg_fmd.*mode = args->*mode;

  error = _make_connection(BSC_CHG_FMD, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
