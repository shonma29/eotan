/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_org
** Description: -
*/

ER
bsys_wget_org (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wget_org *args = (struct bsc_wget_org *)argp;
  req.param.par_wget_org.wid = args->wid;
  req.param.par_wget_org.*parent = args->*parent;
  req.param.par_wget_org.org = args->org;

  error = _make_connection(BSC_WGET_ORG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
