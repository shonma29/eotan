/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_tim
** Description: �����ƥ���֤�����
*/

ER
bsys_set_tim (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_set_tim *args = (struct bsc_set_tim *)argp;
  req.param.par_set_tim.time = args->time;
  req.param.par_set_tim.*tz = args->*tz;

  error = _make_connection(BSC_SET_TIM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
