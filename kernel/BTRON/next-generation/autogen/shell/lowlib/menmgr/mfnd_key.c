/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mfnd_key
** Description: -
*/

ER
bsys_mfnd_key (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_mfnd_key *args = (struct bsc_mfnd_key *)argp;
  req.param.par_mfnd_key.mid = args->mid;
  req.param.par_mfnd_key.ch = args->ch;

  error = _make_connection(BSC_MFND_KEY, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
