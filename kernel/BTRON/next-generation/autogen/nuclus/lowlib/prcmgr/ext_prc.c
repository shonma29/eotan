/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ext_prc
** Description: 自プロセスの終了
*/

ER
bsys_ext_prc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_ext_prc *args = (struct bsc_ext_prc *)argp;
  req.param.par_ext_prc.exit_code = args->exit_code;

  error = _make_connection(BSC_EXT_PRC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
