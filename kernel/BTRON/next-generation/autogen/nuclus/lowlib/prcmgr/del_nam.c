/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: del_nam
** Description: グローバル名データの削除
*/

ER
bsys_del_nam (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_del_nam *args = (struct bsc_del_nam *)argp;
  req.param.par_del_nam.*name = args->*name;

  error = _make_connection(BSC_DEL_NAM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
