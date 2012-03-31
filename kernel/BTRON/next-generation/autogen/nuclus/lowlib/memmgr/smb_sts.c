/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: smb_sts
** Description: ��ͭ����֥�å����֤μ��Ф�
*/

ER
bsys_smb_sts (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_smb_sts *args = (struct bsc_smb_sts *)argp;
  req.param.par_smb_sts.akey = args->akey;
  req.param.par_smb_sts.*buff = args->*buff;

  error = _make_connection(BSC_SMB_STS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
