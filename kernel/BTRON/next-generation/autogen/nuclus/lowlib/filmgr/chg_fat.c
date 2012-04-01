/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_fat
** Description: ファイルのアクセス属性変更
*/

ER
bsys_chg_fat (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_chg_fat *args = (struct bsc_chg_fat *)argp;
  req.param.par_chg_fat.*lnk = args->*lnk;
  req.param.par_chg_fat.attr = args->attr;

  error = _make_connection(BSC_CHG_FAT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
