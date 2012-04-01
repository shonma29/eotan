/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: def_cup
** Description: クリーンアップハンドラの登録
*/

ER
bsys_def_cup (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_def_cup *args = (struct bsc_def_cup *)argp;
  req.param.par_def_cup.clup_hdr = args->clup_hdr;

  error = _make_connection(BSC_DEF_CUP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
