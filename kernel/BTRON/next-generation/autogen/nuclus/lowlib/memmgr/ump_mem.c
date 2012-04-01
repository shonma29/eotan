/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ump_mem
** Description: メモリ空間のマッピングの削除
*/

ER
bsys_ump_mem (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_ump_mem *args = (struct bsc_ump_mem *)argp;
  req.param.par_ump_mem.*logaddr = args->*logaddr;

  error = _make_connection(BSC_UMP_MEM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
