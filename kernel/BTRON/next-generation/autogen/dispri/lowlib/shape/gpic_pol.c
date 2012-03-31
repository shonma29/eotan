/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_pol
** Description: -
*/

ER
bsys_gpic_pol (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gpic_pol *args = (struct bsc_gpic_pol *)argp;
  req.param.par_gpic_pol.pt = args->pt;
  req.param.par_gpic_pol.*poly = args->*poly;
  req.param.par_gpic_pol.width = args->width;

  error = _make_connection(BSC_GPIC_POL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
