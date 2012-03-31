/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: loc_rec
** Description: レコードのロック
*/

ER
bsys_loc_rec (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_loc_rec *args = (struct bsc_loc_rec *)argp;
  req.param.par_loc_rec.fd = args->fd;
  req.param.par_loc_rec.mode = args->mode;

  error = _make_connection(BSC_LOC_REC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
