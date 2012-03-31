/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: xch_fil
** Description: ファイルの内容交換
*/

ER
bsys_xch_fil (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_xch_fil *args = (struct bsc_xch_fil *)argp;
  req.param.par_xch_fil.fd_1 = args->fd_1;
  req.param.par_xch_fil.fd_2 = args->fd_2;

  error = _make_connection(BSC_XCH_FIL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
