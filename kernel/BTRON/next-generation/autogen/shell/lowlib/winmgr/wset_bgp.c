/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wset_bgp
** Description: -
*/

ER
bsys_wset_bgp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wset_bgp *args = (struct bsc_wset_bgp *)argp;
  req.param.par_wset_bgp.wid = args->wid;
  req.param.par_wset_bgp.*pat = args->*pat;

  error = _make_connection(BSC_WSET_BGP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
