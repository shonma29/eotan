/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_str
** Description: -
*/

ER
bsys_gdra_str (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gdra_str *args = (struct bsc_gdra_str *)argp;
  req.param.par_gdra_str.gid = args->gid;
  req.param.par_gdra_str.*str = args->*str;
  req.param.par_gdra_str.len = args->len;
  req.param.par_gdra_str.mode = args->mode;

  error = _make_connection(BSC_GDRA_STR, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
