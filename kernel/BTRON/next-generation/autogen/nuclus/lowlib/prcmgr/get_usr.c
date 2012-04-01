/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_usr
** Description: プロセスのユーザ情報の取り出し
*/

ER
bsys_get_usr (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_usr *args = (struct bsc_get_usr *)argp;
  req.param.par_get_usr.pid = args->pid;
  req.param.par_get_usr.*buff = args->*buff;

  error = _make_connection(BSC_GET_USR, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
