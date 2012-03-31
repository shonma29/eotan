/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_lnk
** Description: �ե�����ؤΥ�󥯤μ��Ф�
*/

ER
bsys_get_lnk (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_lnk *args = (struct bsc_get_lnk *)argp;
  req.param.par_get_lnk.*path = args->*path;
  req.param.par_get_lnk.*lnk = args->*lnk;
  req.param.par_get_lnk.mode = args->mode;

  error = _make_connection(BSC_GET_LNK, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
