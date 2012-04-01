/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_krp
** Description: キーリピート間隔の取り出し
*/

ER
bsys_get_krp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_krp *args = (struct bsc_get_krp *)argp;
  req.param.par_get_krp.*offset = args->*offset;
  req.param.par_get_krp.*interval = args->*interval;

  error = _make_connection(BSC_GET_KRP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
