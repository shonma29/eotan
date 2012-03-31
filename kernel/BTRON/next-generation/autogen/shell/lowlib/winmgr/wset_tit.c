/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wset_tit
** Description: -
*/

ER
bsys_wset_tit (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wset_tit *args = (struct bsc_wset_tit *)argp;
  req.param.par_wset_tit.wid = args->wid;
  req.param.par_wset_tit.pict = args->pict;
  req.param.par_wset_tit.*title = args->*title;
  req.param.par_wset_tit.mode = args->mode;

  error = _make_connection(BSC_WSET_TIT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
