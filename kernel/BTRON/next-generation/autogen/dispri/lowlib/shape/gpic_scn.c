/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_scn
** Description: -
*/

ER
bsys_gpic_scn (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gpic_scn *args = (struct bsc_gpic_scn *)argp;
  req.param.par_gpic_scn.pt = args->pt;
  req.param.par_gpic_scn.*scan = args->*scan;

  error = _make_connection(BSC_GPIC_SCN, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
