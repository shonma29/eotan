/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: see_rec
** Description: レコード位置の移動
*/

ER
bsys_see_rec (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_see_rec *args = (struct bsc_see_rec *)argp;
  req.param.par_see_rec.fd = args->fd;
  req.param.par_see_rec.offset = args->offset;
  req.param.par_see_rec.mode = args->mode;
  req.param.par_see_rec.recnum = args->recnum;

  error = _make_connection(BSC_SEE_REC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
