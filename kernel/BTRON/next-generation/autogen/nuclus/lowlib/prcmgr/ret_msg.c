/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ret_msg
** Description: ��å������ϥ�ɥ�ν�λ
*/

ER
bsys_ret_msg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_ret_msg *args = (struct bsc_ret_msg *)argp;
  req.param.par_ret_msg.ret = args->ret;

  error = _make_connection(BSC_RET_MSG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
