/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cls_dev
** Description: デバイスのクローズ
*/

ER
bsys_cls_dev (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_cls_dev *args = (struct bsc_cls_dev *)argp;
  req.param.par_cls_dev.dd = args->dd;
  req.param.par_cls_dev.eject = args->eject;
  req.param.par_cls_dev.*error = args->*error;

  error = _make_connection(BSC_CLS_DEV, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
