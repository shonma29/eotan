/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gopn_mem
** Description: -
*/

ER
bsys_gopn_mem (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gopn_mem *args = (struct bsc_gopn_mem *)argp;
  req.param.par_gopn_mem.*dev = args->*dev;
  req.param.par_gopn_mem.bmap = args->bmap;
  req.param.par_gopn_mem.*param = args->*param;

  error = _make_connection(BSC_GOPN_MEM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
