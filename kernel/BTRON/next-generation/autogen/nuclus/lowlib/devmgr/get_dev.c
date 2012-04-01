/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_dev
** Description: 論理デバイス名の取り出し
*/

ER
bsys_get_dev (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_dev *args = (struct bsc_get_dev *)argp;
  req.param.par_get_dev.*dev = args->*dev;
  req.param.par_get_dev.num = args->num;

  error = _make_connection(BSC_GET_DEV, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
