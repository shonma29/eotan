/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wlst_wnd
** Description: -
*/

ER
bsys_wlst_wnd (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wlst_wnd *args = (struct bsc_wlst_wnd *)argp;
  req.param.par_wlst_wnd.wid = args->wid;
  req.param.par_wlst_wnd.size = args->size;
  req.param.par_wlst_wnd.*wids = args->*wids;

  error = _make_connection(BSC_WLST_WND, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
