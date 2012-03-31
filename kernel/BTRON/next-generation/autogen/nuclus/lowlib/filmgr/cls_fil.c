/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cls_fil
** Description: ファイルのクローズ
*/

ER
bsys_cls_fil (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_cls_fil *args = (struct bsc_cls_fil *)argp;
  req.param.par_cls_fil.fd = args->fd;

  error = _make_connection(BSC_CLS_FIL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
