/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wera_wnd
** Description: -
*/

ER
bsys_wera_wnd (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wera_wnd *args = (struct bsc_wera_wnd *)argp;
  req.param.par_wera_wnd.wid = args->wid;
  req.param.par_wera_wnd.r = args->r;

  error = _make_connection(BSC_WERA_WND, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
