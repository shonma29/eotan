/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* sys_task.c -- タスク関係のシステムコール
 *
 * $Id: sys_task.c,v 1.4 2000/04/03 14:37:35 naniwa Exp $
 */

#include <core.h>
#include <api.h>
#include "call_kernel.h"


ER
vcre_tsk (T_CTSK *pk_ctsk, ID *rid)
{
  W	i;
  ER	err;

  for (i = MIN_USERTASKID; i <= MAX_USERTASKID; i++)
    {
      err = cre_tsk (i, pk_ctsk);
      if (err == E_OK)
	{
	  *rid = i;
	  return (E_OK);
	}
    }
  return (E_NOMEM);
}
