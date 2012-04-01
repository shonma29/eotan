/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mdel_men
** Description: -
*/

ER
bsys_mdel_men (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_mdel_men *args = (struct bsc_mdel_men *)argp;
  req.param.par_mdel_men.mid = args->mid;

  error = _make_connection(BSC_MDEL_MEN, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
