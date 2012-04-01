/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wset_org
** Description: -
*/

ER
bsys_wset_org (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wset_org *args = (struct bsc_wset_org *)argp;
  req.param.par_wset_org.wid = args->wid;
  req.param.par_wset_org.parent = args->parent;
  req.param.par_wset_org.org = args->org;

  error = _make_connection(BSC_WSET_ORG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
