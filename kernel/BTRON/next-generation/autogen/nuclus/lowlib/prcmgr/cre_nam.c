/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_nam
** Description: グローバル名データの生成
*/

ER
bsys_cre_nam (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_cre_nam *args = (struct bsc_cre_nam *)argp;
  req.param.par_cre_nam.*name = args->*name;
  req.param.par_cre_nam.data = args->data;
  req.param.par_cre_nam.opt = args->opt;

  error = _make_connection(BSC_CRE_NAM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
