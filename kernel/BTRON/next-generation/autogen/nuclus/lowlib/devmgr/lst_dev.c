/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lst_dev
** Description: 登録済みデバイスの取り出し
*/

ER
bsys_lst_dev (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_lst_dev *args = (struct bsc_lst_dev *)argp;
  req.param.par_lst_dev.*dev = args->*dev;
  req.param.par_lst_dev.ndev = args->ndev;

  error = _make_connection(BSC_LST_DEV, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
