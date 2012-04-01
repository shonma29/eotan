/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_ftm
** Description: ファイル日時の変更
*/

ER
bsys_chg_ftm (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_chg_ftm *args = (struct bsc_chg_ftm *)argp;
  req.param.par_chg_ftm.*lnk = args->*lnk;
  req.param.par_chg_ftm.*times = args->*times;

  error = _make_connection(BSC_CHG_FTM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
