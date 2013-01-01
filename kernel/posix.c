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
  ER errno;

  req.param.par_kill.pid = pid;
  req.receive_port = 0; /* 返事は受け取らない */
  req.msg_length = sizeof (struct posix_request);
  req.operation = PSC_KILL;
  req.procid = pid;
  req.caller = KERNEL_TASK; /* KERNEL_TASK で実行 */

  errno = message_send_nowait(PORT_FS, sizeof (struct posix_request), &req);
  return(errno);
}
