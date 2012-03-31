/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_dfm
** Description: デフォルト・アクセスモードの設定
*/

ER
bsys_set_dfm (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_set_dfm *args = (struct bsc_set_dfm *)argp;
  req.param.par_set_dfm.*mode = args->*mode;

  error = _make_connection(BSC_SET_DFM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
