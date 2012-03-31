/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_fnm
** Description: �ե�����̾���ѹ�
*/

ER
bsys_chg_fnm (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_chg_fnm *args = (struct bsc_chg_fnm *)argp;
  req.param.par_chg_fnm.*lnk = args->*lnk;
  req.param.par_chg_fnm.*name = args->*name;

  error = _make_connection(BSC_CHG_FNM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
