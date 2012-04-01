/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_chh
** Description: -
*/

ER
bsys_gget_chh (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gget_chh *args = (struct bsc_gget_chh *)argp;
  req.param.par_gget_chh.gid = args->gid;
  req.param.par_gget_chh.ch = args->ch;

  error = _make_connection(BSC_GGET_CHH, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
