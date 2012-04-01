/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_tit
** Description: -
*/

ER
bsys_wget_tit (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wget_tit *args = (struct bsc_wget_tit *)argp;
  req.param.par_wget_tit.wid = args->wid;
  req.param.par_wget_tit.*pict = args->*pict;
  req.param.par_wget_tit.*title = args->*title;

  error = _make_connection(BSC_WGET_TIT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
