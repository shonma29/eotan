/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: syn_lnk
** Description: リンクファイルの同期
*/

ER
bsys_syn_lnk (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_syn_lnk *args = (struct bsc_syn_lnk *)argp;
  req.param.par_syn_lnk.*lnk = args->*lnk;
  req.param.par_syn_lnk.opt = args->opt;

  error = _make_connection(BSC_SYN_LNK, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
