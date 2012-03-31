/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: att_fls
** Description: ファイルシステムの接続
*/

ER
bsys_att_fls (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_att_fls *args = (struct bsc_att_fls *)argp;
  req.param.par_att_fls.*dev = args->*dev;
  req.param.par_att_fls.*name = args->*name;
  req.param.par_att_fls.*lnk = args->*lnk;
  req.param.par_att_fls.r_only = args->r_only;

  error = _make_connection(BSC_ATT_FLS, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
