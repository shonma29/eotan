/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_pdp
** Description: PD���֤�����
*/

ER
bsys_set_pdp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_set_pdp *args = (struct bsc_set_pdp *)argp;
  req.param.par_set_pdp.pos = args->pos;

  error = _make_connection(BSC_SET_PDP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
