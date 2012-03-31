/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_lmb
** Description: ローカルメモリブロックの獲得
*/

ER
bsys_get_lmb (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_lmb *args = (struct bsc_get_lmb *)argp;
  req.param.par_get_lmb.**mptr = args->**mptr;
  req.param.par_get_lmb.size = args->size;
  req.param.par_get_lmb.opt = args->opt;

  error = _make_connection(BSC_GET_LMB, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
