/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_chr
** Description: -
*/

ER
bsys_gdra_chr (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gdra_chr *args = (struct bsc_gdra_chr *)argp;
  req.param.par_gdra_chr.gid = args->gid;
  req.param.par_gdra_chr.ch = args->ch;
  req.param.par_gdra_chr.mode = args->mode;

  error = _make_connection(BSC_GDRA_CHR, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
