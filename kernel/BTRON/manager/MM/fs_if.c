/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#) $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/manager/MM/fs_if.c,v 1.2 1997/06/29 15:44:31 night Exp $ */

static char rcs[] = "@(#) $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/manager/MM/fs_if.c,v 1.2 1997/06/29 15:44:31 night Exp $";


/*
 * $Log: fs_if.c,v $
 * Revision 1.2  1997/06/29 15:44:31  night
 * �����Ȥ��ѹ�
 *
 * �����������ɤ� SJIS ���� EUC ���ѹ�������
 * ��RCS �� Log �ޥ�������Υ����Ȥ��ɲä���(�ޤ�����Ƥ��ʤ��ä��ե�����Τ�)��
 *
 *
 */


#include "memory_manager.h"


/* �ե����륷���ƥ�ȤΥ��󥿥ե��������갷���ؿ���
 */


ER
open_file (B *fname, W option, W *handle)
{
  return (EMM_NOSUP);
}


ER
close_file (W handle)
{
  return (EMM_NOSUP);
}

read_file (W handle, UW offset, UW size, VP buf)
{
  return (EMM_NOSUP);
}

write_file (W handle, UW offset, UW size, VP buf)
{
  return (EMM_NOSUP);
}
