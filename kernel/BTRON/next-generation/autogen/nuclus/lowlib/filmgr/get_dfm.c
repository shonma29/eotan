/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_dfm
** Description: �ǥե���ȡ����������⡼�ɤμ��Ф�
*/

ER
bsys_get_dfm (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_dfm *args = (struct bsc_get_dfm *)argp;
  req.param.par_get_dfm.*mode = args->*mode;

  error = _make_connection(BSC_GET_DFM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
