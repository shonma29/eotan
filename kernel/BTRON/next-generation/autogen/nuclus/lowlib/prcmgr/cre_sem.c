/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: cre_sem
** Description: セマフォの生成
*/

ER
bsys_cre_sem (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_cre_sem *args = (struct bsc_cre_sem *)argp;
  req.param.par_cre_sem.sem_id = args->sem_id;
  req.param.par_cre_sem.cnt = args->cnt;
  req.param.par_cre_sem.opt = args->opt;

  error = _make_connection(BSC_CRE_SEM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
