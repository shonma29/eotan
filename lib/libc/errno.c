/*

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) 2003, Tomohide Naniwa

*/

#include "../lowlib/lowlib.h"

int errno;

/* 
   lowlib.h の ERRNO マクロを修正した場合には，下記のコードで生成される
   アセンブラコードを用いて call_posix.S を修正する．
 */
#ifdef notdef
static void set_errno()
{
  errno = -ERRNO;
}
#endif
