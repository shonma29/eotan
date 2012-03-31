/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mchg_gat
** Description: -
*/

ER
bsys_mchg_gat (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_mchg_gat *args = (struct bsc_mchg_gat *)argp;
  req.param.par_mchg_gat.mid = args->mid;
  req.param.par_mchg_gat.num = args->num;
  req.param.par_mchg_gat.mode = args->mode;

  error = _make_connection(BSC_MCHG_GAT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
