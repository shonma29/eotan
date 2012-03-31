/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: del_rec
** Description: �쥳���ɤκ��
*/

ER
bsys_del_rec (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_del_rec *args = (struct bsc_del_rec *)argp;
  req.param.par_del_rec.fd = args->fd;

  error = _make_connection(BSC_DEL_REC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
