/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_wrk
** Description: ��ȥե�������ѹ�
*/

ER
bsys_chg_wrk (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_chg_wrk *args = (struct bsc_chg_wrk *)argp;
  req.param.par_chg_wrk.*lnk = args->*lnk;

  error = _make_connection(BSC_CHG_WRK, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
