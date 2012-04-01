/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_prc
** Description: プロセスの生成
*/

ER
bsys_cre_prc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_cre_prc *args = (struct bsc_cre_prc *)argp;
  req.param.par_cre_prc.*lnk = args->*lnk;
  req.param.par_cre_prc.pri = args->pri;
  req.param.par_cre_prc.*msg = args->*msg;

  error = _make_connection(BSC_CRE_PRC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
