/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gen_fil
** Description: �ե������ľ������
*/

ER
bsys_gen_fil (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gen_fil *args = (struct bsc_gen_fil *)argp;
  req.param.par_gen_fil.*lnk = args->*lnk;
  req.param.par_gen_fil.*ref = args->*ref;
  req.param.par_gen_fil.opt = args->opt;

  error = _make_connection(BSC_GEN_FIL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
