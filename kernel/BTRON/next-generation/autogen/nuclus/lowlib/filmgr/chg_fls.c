/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_fls
** Description: ファイルシステム情報の変更
*/

ER
bsys_chg_fls (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_chg_fls *args = (struct bsc_chg_fls *)argp;
  req.param.par_chg_fls.*dev = args->*dev;
  req.param.par_chg_fls.*fs_name = args->*fs_name;
  req.param.par_chg_fls.*fs_locate = args->*fs_locate;

  error = _make_connection(BSC_CHG_FLS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
