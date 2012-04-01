/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: get_ptr
** Description: メモリポインタの獲得
*/

ER
bsys_get_ptr (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_get_ptr *args = (struct bsc_get_ptr *)argp;
  req.param.par_get_ptr.*sptr = args->*sptr;
  req.param.par_get_ptr.offset = args->offset;
  req.param.par_get_ptr.**mptr = args->**mptr;
  req.param.par_get_ptr.size = args->size;

  error = _make_connection(BSC_GET_PTR, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
