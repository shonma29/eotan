/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: lod_spg
** Description: システムプログラムのロード
*/

ER
bsys_lod_spg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_lod_spg *args = (struct bsc_lod_spg *)argp;
  req.param.par_lod_spg.*lnk = args->*lnk;
  req.param.par_lod_spg.*arg = args->*arg;
  req.param.par_lod_spg.attr = args->attr;
  req.param.par_lod_spg.info = args->info;

  error = _make_connection(BSC_LOD_SPG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
