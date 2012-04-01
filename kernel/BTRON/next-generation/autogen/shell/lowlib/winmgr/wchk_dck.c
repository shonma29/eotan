/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wchk_dck
** Description: -
*/

ER
bsys_wchk_dck (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wchk_dck *args = (struct bsc_wchk_dck *)argp;
  req.param.par_wchk_dck.first = args->first;

  error = _make_connection(BSC_WCHK_DCK, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
