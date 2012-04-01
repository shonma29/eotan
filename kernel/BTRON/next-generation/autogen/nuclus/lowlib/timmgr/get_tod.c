/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_tod
** Description: 日付時刻の設定/変換
*/

ER
bsys_get_tod (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_tod *args = (struct bsc_get_tod *)argp;
  req.param.par_get_tod.*date_tim = args->*date_tim;
  req.param.par_get_tod.time = args->time;
  req.param.par_get_tod.local = args->local;

  error = _make_connection(BSC_GET_TOD, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
