/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: msel_men
** Description: -
*/

ER
bsys_msel_men (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_msel_men *args = (struct bsc_msel_men *)argp;
  req.param.par_msel_men.mid = args->mid;
  req.param.par_msel_men.pos = args->pos;

  error = _make_connection(BSC_MSEL_MEN, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
