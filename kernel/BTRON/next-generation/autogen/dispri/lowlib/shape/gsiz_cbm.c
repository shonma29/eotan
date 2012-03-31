/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gsiz_cbm
** Description: -
*/

ER
bsys_gsiz_cbm (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gsiz_cbm *args = (struct bsc_gsiz_cbm *)argp;
  req.param.par_gsiz_cbm.gid = args->gid;
  req.param.par_gsiz_cbm.rp = args->rp;
  req.param.par_gsiz_cbm.compac = args->compac;
  req.param.par_gsiz_cbm.size = args->size;

  error = _make_connection(BSC_GSIZ_CBM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
