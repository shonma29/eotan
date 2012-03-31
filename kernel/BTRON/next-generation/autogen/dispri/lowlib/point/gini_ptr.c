/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gini_ptr
** Description: -
*/

ER
bsys_gini_ptr (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gini_ptr *args = (struct bsc_gini_ptr *)argp;
  req.param.par_gini_ptr.void = args->void;

  error = _make_connection(BSC_GINI_PTR, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
