/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_pln
** Description: -
*/

ER
bsys_gpic_pln (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gpic_pln *args = (struct bsc_gpic_pln *)argp;
  req.param.par_gpic_pln.pt = args->pt;
  req.param.par_gpic_pln.*poly = args->*poly;
  req.param.par_gpic_pln.width = args->width;

  error = _make_connection(BSC_GPIC_PLN, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
