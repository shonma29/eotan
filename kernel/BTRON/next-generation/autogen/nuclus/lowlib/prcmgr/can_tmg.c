/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: can_tmg
** Description: タイムアウトメッセージの取り消し
*/

ER
bsys_can_tmg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_can_tmg *args = (struct bsc_can_tmg *)argp;
  req.param.par_can_tmg.void = args->void;

  error = _make_connection(BSC_CAN_TMG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
