/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wopn_iwd
** Description: -
*/

ER
bsys_wopn_iwd (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wopn_iwd *args = (struct bsc_wopn_iwd *)argp;
  req.param.par_wopn_iwd.gid = args->gid;

  error = _make_connection(BSC_WOPN_IWD, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
