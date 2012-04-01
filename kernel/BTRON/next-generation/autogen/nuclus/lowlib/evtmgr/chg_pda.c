/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: chg_pda
** Description: ポインティングデバイス属性の変更
*/

ER
bsys_chg_pda (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_chg_pda *args = (struct bsc_chg_pda *)argp;
  req.param.par_chg_pda.attr = args->attr;

  error = _make_connection(BSC_CHG_PDA, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
