/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: trc_rec
** Description: レコードのサイズ縮小
*/

ER
bsys_trc_rec (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_trc_rec *args = (struct bsc_trc_rec *)argp;
  req.param.par_trc_rec.fd = args->fd;
  req.param.par_trc_rec.size = args->size;

  error = _make_connection(BSC_TRC_REC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
