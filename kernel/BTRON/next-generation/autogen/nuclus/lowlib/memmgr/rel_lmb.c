/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rel_lmb
** Description: ローカルメモリブロックの開放
*/

ER
bsys_rel_lmb (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_rel_lmb *args = (struct bsc_rel_lmb *)argp;
  req.param.par_rel_lmb.*mptr = args->*mptr;

  error = _make_connection(BSC_REL_LMB, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
