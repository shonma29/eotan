/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rel_ptr
** Description: ����ݥ��󥿤γ���
*/

ER
bsys_rel_ptr (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_rel_ptr *args = (struct bsc_rel_ptr *)argp;
  req.param.par_rel_ptr.*mptr = args->*mptr;

  error = _make_connection(BSC_REL_PTR, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
