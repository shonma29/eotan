/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rel_smb
** Description: ��ͭ����֥�å��κ��
*/

ER
bsys_rel_smb (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_rel_smb *args = (struct bsc_rel_smb *)argp;
  req.param.par_rel_smb.akey = args->akey;

  error = _make_connection(BSC_REL_SMB, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
