/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: dev_sts
** Description: デバイスの管理情報の取り出し
*/

ER
bsys_dev_sts (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_dev_sts *args = (struct bsc_dev_sts *)argp;
  req.param.par_dev_sts.*dev = args->*dev;
  req.param.par_dev_sts.*buf = args->*buf;

  error = _make_connection(BSC_DEV_STS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
