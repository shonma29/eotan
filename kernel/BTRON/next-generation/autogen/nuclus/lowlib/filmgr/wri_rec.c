/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wri_rec
** Description: レコードの書き込み
*/

ER
bsys_wri_rec (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wri_rec *args = (struct bsc_wri_rec *)argp;
  req.param.par_wri_rec.fd = args->fd;
  req.param.par_wri_rec.offset = args->offset;
  req.param.par_wri_rec.*buf = args->*buf;
  req.param.par_wri_rec.size = args->size;
  req.param.par_wri_rec.r_size = args->r_size;
  req.param.par_wri_rec.*subtype = args->*subtype;
  req.param.par_wri_rec.units = args->units;

  error = _make_connection(BSC_WRI_REC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
