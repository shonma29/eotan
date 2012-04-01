/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_lnk
** Description: リンクファイルの生成
*/

ER
bsys_cre_lnk (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_cre_lnk *args = (struct bsc_cre_lnk *)argp;
  req.param.par_cre_lnk.*lnk = args->*lnk;
  req.param.par_cre_lnk.*ref = args->*ref;
  req.param.par_cre_lnk.opt = args->opt;

  error = _make_connection(BSC_CRE_LNK, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
