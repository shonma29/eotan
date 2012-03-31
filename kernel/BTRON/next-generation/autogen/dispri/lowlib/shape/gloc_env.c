/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gloc_env
** Description: -
*/

ER
bsys_gloc_env (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gloc_env *args = (struct bsc_gloc_env *)argp;
  req.param.par_gloc_env.gid = args->gid;
  req.param.par_gloc_env.lock = args->lock;

  error = _make_connection(BSC_GLOC_ENV, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
