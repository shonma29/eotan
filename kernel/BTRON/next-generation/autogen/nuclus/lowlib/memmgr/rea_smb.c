/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rea_smb
** Description: 共有メモリブロックの読み込み
*/

ER
bsys_rea_smb (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_rea_smb *args = (struct bsc_rea_smb *)argp;
  req.param.par_rea_smb.akey = args->akey;
  req.param.par_rea_smb.*buff = args->*buff;
  req.param.par_rea_smb.offset = args->offset;
  req.param.par_rea_smb.size = args->size;
  req.param.par_rea_smb.a_size = args->a_size;

  error = _make_connection(BSC_REA_SMB, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
