/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_bar
** Description: -
*/

ER
bsys_wget_bar (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wget_bar *args = (struct bsc_wget_bar *)argp;
  req.param.par_wget_bar.wid = args->wid;
  req.param.par_wget_bar.*rbar = args->*rbar;
  req.param.par_wget_bar.*bbar = args->*bbar;
  req.param.par_wget_bar.*lbar = args->*lbar;

  error = _make_connection(BSC_WGET_BAR, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
