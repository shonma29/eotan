/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_wrk
** Description: -
*/

ER
bsys_wget_wrk (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wget_wrk *args = (struct bsc_wget_wrk *)argp;
  req.param.par_wget_wrk.wid = args->wid;
  req.param.par_wget_wrk.r = args->r;

  error = _make_connection(BSC_WGET_WRK, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
