/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rsz_lmb
** Description: ローカルメモリブロックのサイズ変更
*/

ER
bsys_rsz_lmb (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_rsz_lmb *args = (struct bsc_rsz_lmb *)argp;
  req.param.par_rsz_lmb.**n_mptr = args->**n_mptr;
  req.param.par_rsz_lmb.*mptr = args->*mptr;
  req.param.par_rsz_lmb.size = args->size;
  req.param.par_rsz_lmb.opt = args->opt;

  error = _make_connection(BSC_RSZ_LMB, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
