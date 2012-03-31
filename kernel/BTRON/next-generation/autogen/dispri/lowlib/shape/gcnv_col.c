/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gcnv_col
** Description: -
*/

ER
bsys_gcnv_col (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gcnv_col *args = (struct bsc_gcnv_col *)argp;
  req.param.par_gcnv_col.gid = args->gid;
  req.param.par_gcnv_col.cv = args->cv;
  req.param.par_gcnv_col.pixv = args->pixv;

  error = _make_connection(BSC_GCNV_COL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
