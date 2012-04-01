/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_fnt
** Description: -
*/

ER
bsys_gget_fnt (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gget_fnt *args = (struct bsc_gget_fnt *)argp;
  req.param.par_gget_fnt.gid = args->gid;
  req.param.par_gget_fnt.*fnt = args->*fnt;
  req.param.par_gget_fnt.*inf = args->*inf;

  error = _make_connection(BSC_GGET_FNT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
