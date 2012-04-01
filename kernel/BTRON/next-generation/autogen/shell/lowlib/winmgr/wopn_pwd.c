/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wopn_pwd
** Description: -
*/

ER
bsys_wopn_pwd (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wopn_pwd *args = (struct bsc_wopn_pwd *)argp;
  req.param.par_wopn_pwd.r = args->r;

  error = _make_connection(BSC_WOPN_PWD, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
