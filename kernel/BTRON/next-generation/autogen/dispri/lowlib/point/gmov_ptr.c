/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gmov_ptr
** Description: -
*/

ER
bsys_gmov_ptr (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gmov_ptr *args = (struct bsc_gmov_ptr *)argp;
  req.param.par_gmov_ptr.pos = args->pos;

  error = _make_connection(BSC_GMOV_PTR, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
