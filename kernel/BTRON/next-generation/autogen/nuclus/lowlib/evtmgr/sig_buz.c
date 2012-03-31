/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: sig_buz
** Description: �֥�����ȯ��
*/

ER
bsys_sig_buz (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_sig_buz *args = (struct bsc_sig_buz *)argp;
  req.param.par_sig_buz.type = args->type;

  error = _make_connection(BSC_SIG_BUZ, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
