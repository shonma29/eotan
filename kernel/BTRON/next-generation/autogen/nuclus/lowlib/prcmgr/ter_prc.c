/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ter_prc
** Description: 他プロセスの強制終了
*/

ER
bsys_ter_prc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_ter_prc *args = (struct bsc_ter_prc *)argp;
  req.param.par_ter_prc.pid = args->pid;
  req.param.par_ter_prc.abort_code = args->abort_code;
  req.param.par_ter_prc.opt = args->opt;

  error = _make_connection(BSC_TER_PRC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
