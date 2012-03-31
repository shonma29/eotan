/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rea_rec
** Description: レコードの読み込み
*/

ER
bsys_rea_rec (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_rea_rec *args = (struct bsc_rea_rec *)argp;
  req.param.par_rea_rec.fd = args->fd;
  req.param.par_rea_rec.offset = args->offset;
  req.param.par_rea_rec.*buf = args->*buf;
  req.param.par_rea_rec.size = args->size;
  req.param.par_rea_rec.r_size = args->r_size;
  req.param.par_rea_rec.*subtype = args->*subtype;

  error = _make_connection(BSC_REA_REC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
