/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wchg_dck
** Description: -
*/

ER
bsys_wchg_dck (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wchg_dck *args = (struct bsc_wchg_dck *)argp;
  req.param.par_wchg_dck.time = args->time;

  error = _make_connection(BSC_WCHG_DCK, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
