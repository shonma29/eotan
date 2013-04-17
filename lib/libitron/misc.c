/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/sys_misc.c,v 1.5 2000/07/09 16:37:59 kishida0 Exp $ */

/*
 * $Log: sys_misc.c,v $
 * Revision 1.5  2000/07/09 16:37:59  kishida0
 * add return value type
 *
 * Revision 1.4  1999/11/11 09:52:47  naniwa
 * minor fix
 *
 * Revision 1.3  1999/11/10 10:30:38  naniwa
 * to support execve, etc
 *
 * Revision 1.2  1999/07/30 08:19:59  naniwa
 * add vcpy_stk()
 *
 * Revision 1.1  1999/04/18 17:48:34  monaka
 * Port-manager and libkernel.a is moved to ITRON. I guess it is reasonable. At least they should not be in BTRON/.
 *
 * Revision 1.4  1999/03/16 02:43:56  monaka
 * Minor changes. Fixed wrong comment.
 *
 * Revision 1.3  1997/08/31 14:10:50  night
 * vsys_inf() の追加。
 *
 * Revision 1.2  1996/11/07  12:40:27  night
 * 関数 vsys_msc() の追加。
 *
 * Revision 1.1  1996/11/06  12:38:26  night
 * 最初の登録
 *
 *
 */



#include <itron/types.h>
#include <itron/syscall.h>
#include <itron/errno.h>
#include <mpu/call_kernel.h>
#include <nerve/svcno.h>


/* vsys_inf
 *
 */
ER
vsys_inf (W func, W sub, VP argp)
{
  return ncall(SVC_GET_SYSTEM_INFO, func, sub, argp);
}

/* vcpy_stk */
ER
vcpy_stk (ID src, W esp, W ebp, W ebx, W ecx, W edx, W esi , W edi, ID dst)
{
  return ncall(SVC_MPU_COPY_STACK, src, esp, ebp,
			ebx, ecx, edx, esi , edi, dst);
}

/* vset_cxt */
ER
vset_ctx (ID tid, W eip, B *stackp, W stsize)
{
  return ncall(SVC_MPU_SET_CONTEXT, tid, eip, stackp, stsize);
}

/* vuse_fpu */
ER
vuse_fpu (ID tid)
{
  return ncall(SVC_MPU_USE_FLOAT, tid);
}
