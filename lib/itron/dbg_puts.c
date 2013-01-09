/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* デバッグ用のシステムコール
 *
 */
#include <core.h>
#include <api.h>
#include "call_kernel.h"

ER dbg_puts(B * msg)
{
    return ncall(SYS_DBG_PUTS, msg);
}