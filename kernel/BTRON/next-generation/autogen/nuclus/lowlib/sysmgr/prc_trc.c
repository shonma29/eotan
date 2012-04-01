/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: prc_trc
** Description: デバッグ対象プロセスの制御
*/

ER
bsys_prc_trc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_prc_trc *args = (struct bsc_prc_trc *)argp;
  req.param.par_prc_trc.trace = args->trace;

  error = _make_connection(BSC_PRC_TRC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
