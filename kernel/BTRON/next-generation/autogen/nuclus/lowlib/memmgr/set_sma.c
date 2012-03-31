/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_sma
** Description: ��ͭ����֥�å�°��������
*/

ER
bsys_set_sma (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_set_sma *args = (struct bsc_set_sma *)argp;
  req.param.par_set_sma.akey = args->akey;
  req.param.par_set_sma.attr = args->attr;

  error = _make_connection(BSC_SET_SMA, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
