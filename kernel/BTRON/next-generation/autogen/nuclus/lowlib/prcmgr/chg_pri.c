/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_pri
** Description: �ץ�����ͥ�����ѹ�
*/

ER
bsys_chg_pri (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_chg_pri *args = (struct bsc_chg_pri *)argp;
  req.param.par_chg_pri.pid = args->pid;
  req.param.par_chg_pri.new_pri = args->new_pri;
  req.param.par_chg_pri.opt = args->opt;

  error = _make_connection(BSC_CHG_PRI, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
