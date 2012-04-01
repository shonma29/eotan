/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mset_itm
** Description: -
*/

ER
bsys_mset_itm (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_mset_itm *args = (struct bsc_mset_itm *)argp;
  req.param.par_mset_itm.mid = args->mid;
  req.param.par_mset_itm.pnum = args->pnum;
  req.param.par_mset_itm.*item = args->*item;

  error = _make_connection(BSC_MSET_ITM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
