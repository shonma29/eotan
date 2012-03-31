/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mpl_sts
** Description: ��ͭ����ס�����֤μ��Ф�
*/

ER
bsys_mpl_sts (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_mpl_sts *args = (struct bsc_mpl_sts *)argp;
  req.param.par_mpl_sts.mpl_id = args->mpl_id;
  req.param.par_mpl_sts.*buff = args->*buff;

  error = _make_connection(BSC_MPL_STS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
