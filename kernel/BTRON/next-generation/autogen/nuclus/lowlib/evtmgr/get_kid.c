/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_kid
** Description: �����ܡ���ID�μ��Ф�
*/

ER
bsys_get_kid (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_kid *args = (struct bsc_get_kid *)argp;
  req.param.par_get_kid.*id = args->*id;

  error = _make_connection(BSC_GET_KID, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
