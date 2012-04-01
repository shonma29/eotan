/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_smb
** Description: 共有メモリブロックの獲得
*/

ER
bsys_get_smb (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_smb *args = (struct bsc_get_smb *)argp;
  req.param.par_get_smb.mpl_id = args->mpl_id;
  req.param.par_get_smb.size = args->size;
  req.param.par_get_smb.opt = args->opt;

  error = _make_connection(BSC_GET_SMB, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
