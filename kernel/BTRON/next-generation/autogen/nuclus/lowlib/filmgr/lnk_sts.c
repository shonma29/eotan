/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lnk_sts
** Description: リンクファイル情報の取り出し
*/

ER
bsys_lnk_sts (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_lnk_sts *args = (struct bsc_lnk_sts *)argp;
  req.param.par_lnk_sts.*lnk = args->*lnk;
  req.param.par_lnk_sts.*stat = args->*stat;

  error = _make_connection(BSC_LNK_STS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
