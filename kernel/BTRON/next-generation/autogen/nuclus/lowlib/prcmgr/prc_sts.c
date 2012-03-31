/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: prc_sts
** Description: �ץ������֤μ��Ф�
*/

ER
bsys_prc_sts (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_prc_sts *args = (struct bsc_prc_sts *)argp;
  req.param.par_prc_sts.pid = args->pid;
  req.param.par_prc_sts.*buff = args->*buff;
  req.param.par_prc_sts.*path = args->*path;

  error = _make_connection(BSC_PRC_STS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
