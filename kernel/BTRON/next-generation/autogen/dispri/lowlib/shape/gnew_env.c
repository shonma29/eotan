/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gnew_env
** Description: -
*/

ER
bsys_gnew_env (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gnew_env *args = (struct bsc_gnew_env *)argp;
  req.param.par_gnew_env.gid = args->gid;

  error = _make_connection(BSC_GNEW_ENV, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
