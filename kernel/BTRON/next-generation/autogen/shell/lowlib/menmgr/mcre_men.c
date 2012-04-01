/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mcre_men
** Description: -
*/

ER
bsys_mcre_men (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_mcre_men *args = (struct bsc_mcre_men *)argp;
  req.param.par_mcre_men.nitem = args->nitem;
  req.param.par_mcre_men.*item = args->*item;
  req.param.par_mcre_men.*attr = args->*attr;

  error = _make_connection(BSC_MCRE_MEN, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
