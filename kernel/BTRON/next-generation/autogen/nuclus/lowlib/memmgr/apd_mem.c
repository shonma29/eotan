/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: apd_mem
** Description: メモリ領域の追加
*/

ER
bsys_apd_mem (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_apd_mem *args = (struct bsc_apd_mem *)argp;
  req.param.par_apd_mem.phaddr = args->phaddr;
  req.param.par_apd_mem.size = args->size;

  error = _make_connection(BSC_APD_MEM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
