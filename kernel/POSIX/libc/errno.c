/*

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) 2003, Tomohide Naniwa

*/

#include "../lowlib/lowlib.h"

int errno;

/* 
   lowlib.h �� ERRNO �ޥ�������������ˤϡ������Υ����ɤ����������
   ������֥饳���ɤ��Ѥ��� call_posix.S �������롥
 */
#ifdef notdef
static void set_errno()
{
  errno = -ERRNO;
}
#endif
