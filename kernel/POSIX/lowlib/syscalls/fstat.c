/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/fstat.c,v 1.3 1999/02/19 10:04:33 monaka Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/fstat.c,v 1.3 1999/02/19 10:04:33 monaka Exp $";

/*
 * $Log: fstat.c,v $
 * Revision 1.3  1999/02/19 10:04:33  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.2  1999/02/19 08:59:18  monaka
 * added the parameter structure on.
 *
 * Revision 1.1  1997/08/31 13:10:44  night
 * �ǽ����Ͽ
 *
 *
 *
 */


#include "../lowlib.h"




int
psys_fstat (void *argp)
{
  struct a
    {
      W fd;
      struct stat *st;
    } *args = (struct a *)argp;

  /*
   * �ޤ�����ץ���Ȥ��Ƥ��ʤ���ENOSYS ���֤���
   */
  errno = ENOSYS;

  return (NULL);
}
