/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wcls_wnd
** Description: -
*/

ER
bsys_wcls_wnd (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wcls_wnd *args = (struct bsc_wcls_wnd *)argp;
  req.param.par_wcls_wnd.wid = args->wid;
  req.param.par_wcls_wnd.opt = args->opt;

  error = _make_connection(BSC_WCLS_WND, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
