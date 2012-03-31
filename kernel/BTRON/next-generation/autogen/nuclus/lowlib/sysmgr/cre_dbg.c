/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_dbg
** Description: デバッグ対象プロセスの生成
*/

ER
bsys_cre_dbg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_cre_dbg *args = (struct bsc_cre_dbg *)argp;
  req.param.par_cre_dbg.*lnk = args->*lnk;
  req.param.par_cre_dbg.pri = args->pri;
  req.param.par_cre_dbg.arg = args->arg;
  req.param.par_cre_dbg.dpid = args->dpid;

  error = _make_connection(BSC_CRE_DBG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
