/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: loc_prc
** Description: プロセスのロック/アンロック
*/

ER
bsys_loc_prc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_loc_prc *args = (struct bsc_loc_prc *)argp;
  req.param.par_loc_prc.onoff = args->onoff;

  error = _make_connection(BSC_LOC_PRC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
