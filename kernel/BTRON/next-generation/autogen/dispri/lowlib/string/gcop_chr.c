/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gcop_chr
** Description: -
*/

ER
bsys_gcop_chr (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gcop_chr *args = (struct bsc_gcop_chr *)argp;
  req.param.par_gcop_chr.gid = args->gid;
  req.param.par_gcop_chr.asize = args->asize;
  req.param.par_gcop_chr.isize = args->isize;
  req.param.par_gcop_chr.*img = args->*img;
  req.param.par_gcop_chr.mode = args->mode;

  error = _make_connection(BSC_GCOP_CHR, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
