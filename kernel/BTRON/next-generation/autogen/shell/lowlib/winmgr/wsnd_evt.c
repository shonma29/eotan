/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wsnd_evt
** Description: -
*/

ER
bsys_wsnd_evt (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wsnd_evt *args = (struct bsc_wsnd_evt *)argp;
  req.param.par_wsnd_evt.*evt = args->*evt;

  error = _make_connection(BSC_WSND_EVT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
