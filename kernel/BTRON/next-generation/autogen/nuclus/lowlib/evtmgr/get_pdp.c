/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_pdp
** Description: PD���֤μ��Ф�
*/

ER
bsys_get_pdp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_pdp *args = (struct bsc_get_pdp *)argp;
  req.param.par_get_pdp.*pos = args->*pos;

  error = _make_connection(BSC_GET_PDP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
