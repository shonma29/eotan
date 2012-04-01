/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wai_sem
** Description: セマフォの獲得
*/

ER
bsys_wai_sem (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wai_sem *args = (struct bsc_wai_sem *)argp;
  req.param.par_wai_sem.sem_id = args->sem_id;

  error = _make_connection(BSC_WAI_SEM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
