/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wget_sts
** Description: -
*/

ER
bsys_wget_sts (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wget_sts *args = (struct bsc_wget_sts *)argp;
  req.param.par_wget_sts.wid = args->wid;
  req.param.par_wget_sts.*stat = args->*stat;
  req.param.par_wget_sts.*atr = args->*atr;

  error = _make_connection(BSC_WGET_STS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
