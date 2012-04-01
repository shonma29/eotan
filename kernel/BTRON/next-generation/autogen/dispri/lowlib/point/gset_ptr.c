/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_ptr
** Description: -
*/

ER
bsys_gset_ptr (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gset_ptr *args = (struct bsc_gset_ptr *)argp;
  req.param.par_gset_ptr.style = args->style;
  req.param.par_gset_ptr.*img = args->*img;
  req.param.par_gset_ptr.fgcol = args->fgcol;
  req.param.par_gset_ptr.bgcol = args->bgcol;

  error = _make_connection(BSC_GSET_PTR, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
