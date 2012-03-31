/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ofl_sts
** Description: ファイル情報の取り出し
*/

ER
bsys_ofl_sts (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_ofl_sts *args = (struct bsc_ofl_sts *)argp;
  req.param.par_ofl_sts.fd = args->fd;
  req.param.par_ofl_sts.*name = args->*name;
  req.param.par_ofl_sts.*stat = args->*stat;
  req.param.par_ofl_sts.*locat = args->*locat;

  error = _make_connection(BSC_OFL_STS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
