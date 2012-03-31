/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_smi
** Description: ��ͭ����֥�å��Υ桼������μ��Ф�
*/

ER
bsys_get_smi (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_smi *args = (struct bsc_get_smi *)argp;
  req.param.par_get_smi.akey = args->akey;
  req.param.par_get_smi.info = args->info;

  error = _make_connection(BSC_GET_SMI, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
