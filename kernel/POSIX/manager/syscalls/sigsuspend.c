/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 * $Log: sigsuspend.c,v $
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "posix.h"

W
psc_sigsuspend_f (struct posix_request *req)
{

  /* ここに実際の処理が入る */

  /* とりあえず、サポートしていないというエラーで返す
   */
  put_response (req, EP_NOSUP, 0, 0, 0);

  return (FAIL);
}  
