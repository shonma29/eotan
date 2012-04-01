/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mget_itm
** Description: -
*/

ER
bsys_mget_itm (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_mget_itm *args = (struct bsc_mget_itm *)argp;
  req.param.par_mget_itm.mid = args->mid;
  req.param.par_mget_itm.pnum = args->pnum;
  req.param.par_mget_itm.*item = args->*item;

  error = _make_connection(BSC_MGET_ITM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
