/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_drg
** Description: -
*/

ER
bsys_wget_drg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wget_drg *args = (struct bsc_wget_drg *)argp;
  req.param.par_wget_drg.pos = args->pos;
  req.param.par_wget_drg.*evt = args->*evt;

  error = _make_connection(BSC_WGET_DRG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
