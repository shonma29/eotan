/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: clr_msg
** Description: ��å������Υ��ꥢ
*/

ER
bsys_clr_msg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_clr_msg *args = (struct bsc_clr_msg *)argp;
  req.param.par_clr_msg.t_mask = args->t_mask;
  req.param.par_clr_msg.last_mask = args->last_mask;

  error = _make_connection(BSC_CLR_MSG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
