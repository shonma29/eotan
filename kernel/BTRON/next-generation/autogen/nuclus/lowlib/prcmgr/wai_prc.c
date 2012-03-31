/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wai_prc
** Description: �ץ����λ����Ԥ�
*/

ER
bsys_wai_prc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wai_prc *args = (struct bsc_wai_prc *)argp;
  req.param.par_wai_prc.time = args->time;

  error = _make_connection(BSC_WAI_PRC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
