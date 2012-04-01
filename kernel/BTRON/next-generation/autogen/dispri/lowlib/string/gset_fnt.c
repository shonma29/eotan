/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_fnt
** Description: -
*/

ER
bsys_gset_fnt (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gset_fnt *args = (struct bsc_gset_fnt *)argp;
  req.param.par_gset_fnt.gid = args->gid;
  req.param.par_gset_fnt.*fnt = args->*fnt;

  error = _make_connection(BSC_GSET_FNT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
