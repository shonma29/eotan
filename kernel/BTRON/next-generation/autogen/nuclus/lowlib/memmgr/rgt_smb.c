/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: rgt_smb
** Description: 共有メモリブロックの再獲得
*/

ER
bsys_rgt_smb (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_rgt_smb *args = (struct bsc_rgt_smb *)argp;
  req.param.par_rgt_smb.akey = args->akey;

  error = _make_connection(BSC_RGT_SMB, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
