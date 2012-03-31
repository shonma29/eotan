/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lst_fls
** Description: ファイルシステムのリストアップ
*/

ER
bsys_lst_fls (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_lst_fls *args = (struct bsc_lst_fls *)argp;
  req.param.par_lst_fls.*buff = args->*buff;
  req.param.par_lst_fls.cnt = args->cnt;

  error = _make_connection(BSC_LST_FLS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
