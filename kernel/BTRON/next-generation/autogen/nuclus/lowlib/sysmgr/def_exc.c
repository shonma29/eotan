/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: def_exc
** Description: 例外処理ハンドラの定義
*/

ER
bsys_def_exc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_def_exc *args = (struct bsc_def_exc *)argp;
  req.param.par_def_exc.exc_vec = args->exc_vec;
  req.param.par_def_exc.exc_hdr = args->exc_hdr;

  error = _make_connection(BSC_DEF_EXC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
