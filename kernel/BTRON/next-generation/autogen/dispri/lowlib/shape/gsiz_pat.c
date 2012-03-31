/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gsiz_pat
** Description: -
*/

ER
bsys_gsiz_pat (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gsiz_pat *args = (struct bsc_gsiz_pat *)argp;
  req.param.par_gsiz_pat.gid = args->gid;
  req.param.par_gsiz_pat.kind = args->kind;
  req.param.par_gsiz_pat.hsize = args->hsize;
  req.param.par_gsiz_pat.vsize = args->vsize;
  req.param.par_gsiz_pat.size = args->size;

  error = _make_connection(BSC_GSIZ_PAT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
