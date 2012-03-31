/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wdef_fep
** Description: -
*/

ER
bsys_wdef_fep (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wdef_fep *args = (struct bsc_wdef_fep *)argp;
  req.param.par_wdef_fep.onoff = args->onoff;

  error = _make_connection(BSC_WDEF_FEP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
