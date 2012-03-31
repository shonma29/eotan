/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: set_smi
** Description: 共有メモリブロックのユーザ情報の設定
*/

ER
bsys_set_smi (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_set_smi *args = (struct bsc_set_smi *)argp;
  req.param.par_set_smi.akey = args->akey;
  req.param.par_set_smi.attr = args->attr;

  error = _make_connection(BSC_SET_SMI, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
