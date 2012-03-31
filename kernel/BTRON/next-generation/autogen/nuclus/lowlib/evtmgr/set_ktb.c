/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_ktb
** Description: 文字コード変換表の設定
*/

ER
bsys_set_ktb (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_set_ktb *args = (struct bsc_set_ktb *)argp;
  req.param.par_set_ktb.*keytab = args->*keytab;

  error = _make_connection(BSC_SET_KTB, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
