/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: del_mpl
** Description: メモリプールの削除
*/

ER
bsys_del_mpl (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_del_mpl *args = (struct bsc_del_mpl *)argp;
  req.param.par_del_mpl.mpl_id = args->mpl_id;

  error = _make_connection(BSC_DEL_MPL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
