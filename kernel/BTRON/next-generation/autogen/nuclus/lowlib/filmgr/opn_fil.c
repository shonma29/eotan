/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: opn_fil
** Description: ファイルのオープン
*/

ER
bsys_opn_fil (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_opn_fil *args = (struct bsc_opn_fil *)argp;
  req.param.par_opn_fil.*lnk = args->*lnk;
  req.param.par_opn_fil.o_mode = args->o_mode;
  req.param.par_opn_fil.*pwd = args->*pwd;

  error = _make_connection(BSC_OPN_FIL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
