/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gtst_pnt
** Description: -
*/

ER
bsys_gtst_pnt (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gtst_pnt *args = (struct bsc_gtst_pnt *)argp;
  req.param.par_gtst_pnt.gid = args->gid;
  req.param.par_gtst_pnt.p = args->p;
  req.param.par_gtst_pnt.val = args->val;

  error = _make_connection(BSC_GTST_PNT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
