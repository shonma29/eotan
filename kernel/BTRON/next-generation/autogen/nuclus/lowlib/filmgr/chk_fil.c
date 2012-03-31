/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chk_fil
** Description: ファイルのアクセス権チェック
*/

ER
bsys_chk_fil (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_chk_fil *args = (struct bsc_chk_fil *)argp;
  req.param.par_chk_fil.*lnk = args->*lnk;
  req.param.par_chk_fil.mode = args->mode;
  req.param.par_chk_fil.*pwd = args->*pwd;

  error = _make_connection(BSC_CHK_FIL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
