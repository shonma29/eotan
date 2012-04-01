/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: unl_spg
** Description: システムプログラムのアンロード
*/

ER
bsys_unl_spg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_unl_spg *args = (struct bsc_unl_spg *)argp;
  req.param.par_unl_spg.prog_id = args->prog_id;

  error = _make_connection(BSC_UNL_SPG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
