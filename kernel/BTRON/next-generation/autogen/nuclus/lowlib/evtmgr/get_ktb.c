/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_ktb
** Description: ʸ���������Ѵ�ɽ�μ��Ф�
*/

ER
bsys_get_ktb (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_ktb *args = (struct bsc_get_ktb *)argp;
  req.param.par_get_ktb.*keytab = args->*keytab;

  error = _make_connection(BSC_GET_KTB, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
