/*

  GNU GENERAL PUBLIC LICENSE
  Version 2, June 1991

  (C) 2001-2002, Tomohide Naniwa

*/

#include <services.h>
#include "core.h"
#include "lowlib.h"
#include "thread.h"
#include "func.h"
#include "../servers/fs/posix.h"

ER posix_kill_proc(ID pid)
{
  struct posix_request	req;
  ER_UINT rsize;

  req.param.par_kill.pid = pid;
  req.msg_length = sizeof (struct posix_request);
  req.operation = PSC_KILL;
  req.procid = pid;
  req.caller = KERNEL_TASK; /* KERNEL_TASK で実行 */

  rsize = port_call(PORT_FS, 0xffffffff, &req, sizeof (struct posix_request));
  return(rsize);
}
