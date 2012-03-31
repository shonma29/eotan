/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wscr_wnd
** Description: -
*/

ER
bsys_wscr_wnd (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wscr_wnd *args = (struct bsc_wscr_wnd *)argp;
  req.param.par_wscr_wnd.wid = args->wid;
  req.param.par_wscr_wnd.r = args->r;
  req.param.par_wscr_wnd.dh = args->dh;
  req.param.par_wscr_wnd.dv = args->dv;
  req.param.par_wscr_wnd.mode = args->mode;

  error = _make_connection(BSC_WSCR_WND, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
