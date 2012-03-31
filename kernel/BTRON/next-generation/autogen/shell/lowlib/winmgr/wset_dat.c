/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wset_dat
** Description: -
*/

ER
bsys_wset_dat (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wset_dat *args = (struct bsc_wset_dat *)argp;
  req.param.par_wset_dat.wid = args->wid;
  req.param.par_wset_dat.dat = args->dat;

  error = _make_connection(BSC_WSET_DAT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
