/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_krm
** Description: 自動リピート対象キーの設定
*/

ER
bsys_set_krm (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_set_krm *args = (struct bsc_set_krm *)argp;
  req.param.par_set_krm.keymap = args->keymap;

  error = _make_connection(BSC_SET_KRM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
