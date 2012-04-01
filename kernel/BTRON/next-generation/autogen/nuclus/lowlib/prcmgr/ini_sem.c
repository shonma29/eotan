/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: ini_sem
** Description: セマフォの再初期化
*/

ER
bsys_ini_sem (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_ini_sem *args = (struct bsc_ini_sem *)argp;
  req.param.par_ini_sem.sem_id = args->sem_id;
  req.param.par_ini_sem.cnt = args->cnt;

  error = _make_connection(BSC_INI_SEM, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
