/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_etm
** Description: イベントタイマーの取り出し
*/

ER
bsys_get_etm (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_etm *args = (struct bsc_get_etm *)argp;
  req.param.par_get_etm.time = args->time;

  error = _make_connection(BSC_GET_ETM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
