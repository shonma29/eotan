/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: smb_key
** Description: ��ͭ����֥�å����������������μ��Ф�
*/

ER
bsys_smb_key (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_smb_key *args = (struct bsc_smb_key *)argp;
  req.param.par_smb_key.*maddr = args->*maddr;

  error = _make_connection(BSC_SMB_KEY, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
