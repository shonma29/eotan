/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wchg_ful
** Description: -
*/

ER
bsys_wchg_ful (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wchg_ful *args = (struct bsc_wchg_ful *)argp;
  req.param.par_wchg_ful.void = args->void;

  error = _make_connection(BSC_WCHG_FUL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
