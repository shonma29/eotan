/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: fls_sts
** Description: ファイルシステムの管理情報取り出し
*/

ER
bsys_fls_sts (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_fls_sts *args = (struct bsc_fls_sts *)argp;
  req.param.par_fls_sts.*dev = args->*dev;
  req.param.par_fls_sts.*buff = args->*buff;

  error = _make_connection(BSC_FLS_STS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
