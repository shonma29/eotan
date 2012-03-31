/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_bgp
** Description: -
*/

ER
bsys_wget_bgp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wget_bgp *args = (struct bsc_wget_bgp *)argp;
  req.param.par_wget_bgp.wid = args->wid;
  req.param.par_wget_bgp.*pat = args->*pat;
  req.param.par_wget_bgp.size = args->size;

  error = _make_connection(BSC_WGET_BGP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
