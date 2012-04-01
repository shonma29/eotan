/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_dat
** Description: -
*/

ER
bsys_wget_dat (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wget_dat *args = (struct bsc_wget_dat *)argp;
  req.param.par_wget_dat.wid = args->wid;
  req.param.par_wget_dat.dat = args->dat;

  error = _make_connection(BSC_WGET_DAT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
