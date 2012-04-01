/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: map_mem
** Description: メモリ空間のマッピング
*/

ER
bsys_map_mem (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_map_mem *args = (struct bsc_map_mem *)argp;
  req.param.par_map_mem.phaddr = args->phaddr;
  req.param.par_map_mem.**logaddr = args->**logaddr;
  req.param.par_map_mem.size = args->size;
  req.param.par_map_mem.opt = args->opt;

  error = _make_connection(BSC_MAP_MEM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
