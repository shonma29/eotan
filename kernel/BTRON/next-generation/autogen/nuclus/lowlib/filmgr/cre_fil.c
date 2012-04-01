/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_fil
** Description: ファイルの生成
*/

ER
bsys_cre_fil (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_cre_fil *args = (struct bsc_cre_fil *)argp;
  req.param.par_cre_fil.*lnk = args->*lnk;
  req.param.par_cre_fil.*name = args->*name;
  req.param.par_cre_fil.*mode = args->*mode;
  req.param.par_cre_fil.atype = args->atype;
  req.param.par_cre_fil.opt = args->opt;

  error = _make_connection(BSC_CRE_FIL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
