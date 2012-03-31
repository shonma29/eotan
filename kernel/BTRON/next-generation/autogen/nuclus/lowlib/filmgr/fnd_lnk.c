/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: fnd_lnk
** Description: リンクレコードの検索
*/

ER
bsys_fnd_lnk (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_fnd_lnk *args = (struct bsc_fnd_lnk *)argp;
  req.param.par_fnd_lnk.fd = args->fd;
  req.param.par_fnd_lnk.mode = args->mode;
  req.param.par_fnd_lnk.*lnk = args->*lnk;
  req.param.par_fnd_lnk.subtype = args->subtype;
  req.param.par_fnd_lnk.recnum = args->recnum;

  error = _make_connection(BSC_FND_LNK, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
