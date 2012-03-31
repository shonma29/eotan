/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: del_fil
** Description: �ե�����κ��
*/

ER
bsys_del_fil (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_del_fil *args = (struct bsc_del_fil *)argp;
  req.param.par_del_fil.*org = args->*org;
  req.param.par_del_fil.*lnk = args->*lnk;
  req.param.par_del_fil.force = args->force;

  error = _make_connection(BSC_DEL_FIL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
