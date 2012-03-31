/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_sma
** Description: ��ͭ����֥�å�°���μ��Ф�
*/

ER
bsys_get_sma (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_sma *args = (struct bsc_get_sma *)argp;
  req.param.par_get_sma.akey = args->akey;

  error = _make_connection(BSC_GET_SMA, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
