/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_col
** Description: -
*/

ER
bsys_gget_col (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gget_col *args = (struct bsc_gget_col *)argp;
  req.param.par_gget_col.gid = args->gid;
  req.param.par_gget_col.p = args->p;
  req.param.par_gget_col.*cv = args->*cv;
  req.param.par_gget_col.cnt = args->cnt;

  error = _make_connection(BSC_GGET_COL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
