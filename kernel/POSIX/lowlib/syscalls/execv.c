/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/execv.c,v 1.2 1999/03/21 00:57:23 monaka Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/execv.c,v 1.2 1999/03/21 00:57:23 monaka Exp $";

/*
 * $Log: execv.c,v $
 * Revision 1.2  1999/03/21 00:57:23  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.1  1997/08/31 13:10:43  night
 * �ǽ����Ͽ
 *
 *
 *
 */


#include "../lowlib.h"




int
psys_execv (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	res;
  struct psc_execv *args = (struct psc_execv *)argp;

  /*
   * �ޤ�����ץ���Ȥ��Ƥ��ʤ���ENOSYS ���֤���
   */
  errno = ENOSYS;

  return (NULL);
}
