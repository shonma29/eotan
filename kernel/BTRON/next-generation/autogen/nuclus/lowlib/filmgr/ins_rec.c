/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ins_rec
** Description: レコードの挿入
*/

ER
bsys_ins_rec (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_ins_rec *args = (struct bsc_ins_rec *)argp;
  req.param.par_ins_rec.fd = args->fd;
  req.param.par_ins_rec.*buf = args->*buf;
  req.param.par_ins_rec.size = args->size;
  req.param.par_ins_rec.type = args->type;
  req.param.par_ins_rec.subtype = args->subtype;
  req.param.par_ins_rec.units = args->units;

  error = _make_connection(BSC_INS_REC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
