/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: del_fls
** Description: ファイルシステムの切断
*/

ER
bsys_del_fls (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_del_fls *args = (struct bsc_del_fls *)argp;
  req.param.par_del_fls.*dev = args->*dev;
  req.param.par_del_fls.eject = args->eject;

  error = _make_connection(BSC_DEL_FLS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
