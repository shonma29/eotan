/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wswi_wnd
** Description: -
*/

ER
bsys_wswi_wnd (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wswi_wnd *args = (struct bsc_wswi_wnd *)argp;
  req.param.par_wswi_wnd.wid = args->wid;
  req.param.par_wswi_wnd.*evt = args->*evt;

  error = _make_connection(BSC_WSWI_WND, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
