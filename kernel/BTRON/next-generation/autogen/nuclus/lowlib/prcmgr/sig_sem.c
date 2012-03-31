/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: sig_sem
** Description: セマフォの開放
*/

ER
bsys_sig_sem (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_sig_sem *args = (struct bsc_sig_sem *)argp;
  req.param.par_sig_sem.sem_id = args->sem_id;
  req.param.par_sig_sem.cnt = args->cnt;

  error = _make_connection(BSC_SIG_SEM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
