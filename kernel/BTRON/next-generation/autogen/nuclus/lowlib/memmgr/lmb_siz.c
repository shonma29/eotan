/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lmb_siz
** Description: ローカルメモリブロック・サイズの取り出し
*/

ER
bsys_lmb_siz (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_lmb_siz *args = (struct bsc_lmb_siz *)argp;
  req.param.par_lmb_siz.*mptr = args->*mptr;
  req.param.par_lmb_siz.size = args->size;

  error = _make_connection(BSC_LMB_SIZ, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
