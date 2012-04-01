/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_spl
** Description: -
*/

ER
bsys_gpic_spl (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gpic_spl *args = (struct bsc_gpic_spl *)argp;
  req.param.par_gpic_spl.p = args->p;
  req.param.par_gpic_spl.np = args->np;
  req.param.par_gpic_spl.pt = args->pt;
  req.param.par_gpic_spl.width = args->width;

  error = _make_connection(BSC_GPIC_SPL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
