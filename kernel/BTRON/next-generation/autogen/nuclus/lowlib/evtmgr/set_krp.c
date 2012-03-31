/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_krp
** Description: ������ԡ��ȴֳ֤�����
*/

ER
bsys_set_krp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_set_krp *args = (struct bsc_set_krp *)argp;
  req.param.par_set_krp.offset = args->offset;
  req.param.par_set_krp.interval = args->interval;

  error = _make_connection(BSC_SET_KRP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
