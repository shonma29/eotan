/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rcr_prc
** Description: プロセスの再生成
*/

ER
bsys_rcr_prc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_rcr_prc *args = (struct bsc_rcr_prc *)argp;
  req.param.par_rcr_prc.*lnk = args->*lnk;
  req.param.par_rcr_prc.*msg = args->*msg;

  error = _make_connection(BSC_RCR_PRC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
