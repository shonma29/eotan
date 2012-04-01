/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_mpl
** Description: メモリプールの生成
*/

ER
bsys_cre_mpl (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_cre_mpl *args = (struct bsc_cre_mpl *)argp;
  req.param.par_cre_mpl.mpl_id = args->mpl_id;
  req.param.par_cre_mpl.size = args->size;
  req.param.par_cre_mpl.attr = args->attr;

  error = _make_connection(BSC_CRE_MPL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
