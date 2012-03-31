/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: req_tmg
** Description: タイムアウトメッセージの要求
*/

ER
bsys_req_tmg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_req_tmg *args = (struct bsc_req_tmg *)argp;
  req.param.par_req_tmg.time = args->time;
  req.param.par_req_tmg.code = args->code;

  error = _make_connection(BSC_REQ_TMG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
