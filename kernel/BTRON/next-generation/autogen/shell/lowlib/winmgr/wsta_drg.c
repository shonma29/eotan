/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wsta_drg
** Description: -
*/

ER
bsys_wsta_drg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wsta_drg *args = (struct bsc_wsta_drg *)argp;
  req.param.par_wsta_drg.wid = args->wid;
  req.param.par_wsta_drg.lock = args->lock;

  error = _make_connection(BSC_WSTA_DRG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
