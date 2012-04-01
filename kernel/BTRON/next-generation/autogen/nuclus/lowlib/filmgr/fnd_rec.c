/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: fnd_rec
** Description: レコードの検索
*/

ER
bsys_fnd_rec (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_fnd_rec *args = (struct bsc_fnd_rec *)argp;
  req.param.par_fnd_rec.fd = args->fd;
  req.param.par_fnd_rec.mode = args->mode;
  req.param.par_fnd_rec.typemask = args->typemask;
  req.param.par_fnd_rec.subtype = args->subtype;
  req.param.par_fnd_rec.recnum = args->recnum;

  error = _make_connection(BSC_FND_REC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
