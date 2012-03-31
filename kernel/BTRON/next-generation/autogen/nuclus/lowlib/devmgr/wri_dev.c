/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wri_dev
** Description: �ǥХ����ؤΥǡ����񤭹���
*/

ER
bsys_wri_dev (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wri_dev *args = (struct bsc_wri_dev *)argp;
  req.param.par_wri_dev.dd = args->dd;
  req.param.par_wri_dev.start = args->start;
  req.param.par_wri_dev.*buf = args->*buf;
  req.param.par_wri_dev.size = args->size;
  req.param.par_wri_dev.a_size = args->a_size;
  req.param.par_wri_dev.*error = args->*error;

  error = _make_connection(BSC_WRI_DEV, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
