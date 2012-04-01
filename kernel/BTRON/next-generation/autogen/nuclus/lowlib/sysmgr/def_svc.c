/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: def_svc
** Description: システムコールハンドラの定義
*/

ER
bsys_def_svc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_def_svc *args = (struct bsc_def_svc *)argp;
  req.param.par_def_svc.svc_code = args->svc_code;
  req.param.par_def_svc.*svc_hdrs = args->*svc_hdrs;
  req.param.par_def_svc.opt = args->opt;

  error = _make_connection(BSC_DEF_SVC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
