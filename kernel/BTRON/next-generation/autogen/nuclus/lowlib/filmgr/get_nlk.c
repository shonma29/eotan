/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_nlk
** Description: リンクの順次取り出し
*/

ER
bsys_get_nlk (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_nlk *args = (struct bsc_get_nlk *)argp;
  req.param.par_get_nlk.*lnk = args->*lnk;

  error = _make_connection(BSC_GET_NLK, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
