/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_for
** Description: -
*/

ER
bsys_gset_for (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gset_for *args = (struct bsc_gset_for *)argp;
  req.param.par_gset_for.gid = args->gid;
  req.param.par_gset_for.rlp = args->rlp;

  error = _make_connection(BSC_GSET_FOR, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
