/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lmb_sts
** Description: ����������ΰ�ξ��ּ��Ф�
*/

ER
bsys_lmb_sts (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_lmb_sts *args = (struct bsc_lmb_sts *)argp;
  req.param.par_lmb_sts.*buff = args->*buff;

  error = _make_connection(BSC_LMB_STS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
