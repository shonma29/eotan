/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ret_exc
** Description: 例外処理ハンドラの終了
*/

ER
bsys_ret_exc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_ret_exc *args = (struct bsc_ret_exc *)argp;
  req.param.par_ret_exc.ret = args->ret;

  error = _make_connection(BSC_RET_EXC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
