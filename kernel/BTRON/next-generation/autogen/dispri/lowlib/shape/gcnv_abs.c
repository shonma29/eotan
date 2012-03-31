/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gcnv_abs
** Description: -
*/

ER
bsys_gcnv_abs (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gcnv_abs *args = (struct bsc_gcnv_abs *)argp;
  req.param.par_gcnv_abs.gid = args->gid;
  req.param.par_gcnv_abs.pp = args->pp;

  error = _make_connection(BSC_GCNV_ABS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
