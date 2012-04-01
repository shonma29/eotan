/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mchg_dtm
** Description: -
*/

ER
bsys_mchg_dtm (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_mchg_dtm *args = (struct bsc_mchg_dtm *)argp;
  req.param.par_mchg_dtm.time = args->time;

  error = _make_connection(BSC_MCHG_DTM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
