/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wfnd_wnd
** Description: -
*/

ER
bsys_wfnd_wnd (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wfnd_wnd *args = (struct bsc_wfnd_wnd *)argp;
  req.param.par_wfnd_wnd.gpos = args->gpos;
  req.param.par_wfnd_wnd.lpos = args->lpos;
  req.param.par_wfnd_wnd.*wid = args->*wid;

  error = _make_connection(BSC_WFND_WND, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
