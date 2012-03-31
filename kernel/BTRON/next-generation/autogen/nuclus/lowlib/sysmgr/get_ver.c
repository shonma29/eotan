/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_ver
** Description: OS�С���������μ��Ф�
*/

ER
bsys_get_ver (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_ver *args = (struct bsc_get_ver *)argp;
  req.param.par_get_ver.*version = args->*version;

  error = _make_connection(BSC_GET_VER, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
