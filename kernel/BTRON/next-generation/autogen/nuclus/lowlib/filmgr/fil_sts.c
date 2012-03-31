/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: fil_sts
** Description: ファイル情報の取り出し
*/

ER
bsys_fil_sts (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_fil_sts *args = (struct bsc_fil_sts *)argp;
  req.param.par_fil_sts.*lnk = args->*lnk;
  req.param.par_fil_sts.*name = args->*name;
  req.param.par_fil_sts.*stat = args->*stat;
  req.param.par_fil_sts.*locat = args->*locat;

  error = _make_connection(BSC_FIL_STS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
