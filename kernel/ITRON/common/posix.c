/*

  GNU GENERAL PUBLIC LICENSE
  Version 2, June 1991

  (C) 2001-2002, Tomohide Naniwa

*/

#include "itron.h"
#include "errno.h"
#include "lowlib.h"
#include "task.h"
#include "func.h"
#include "../../POSIX/manager/posix.h"

#ifdef notdef
static ID posix_manager = 0;
/* 静的変数の初期化に問題がある */
#else
ID posix_manager;
#endif

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

  if (posix_manager == 0) {
    find_port (POSIX_MANAGER, &posix_manager);
  }
  errno = psnd_mbf (posix_manager, sizeof (struct posix_request), &req);
  return(errno);
}
