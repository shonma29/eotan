/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#) $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/manager/MM/assert.c,v 1.2 1997/06/29 15:44:29 night Exp $ */

static char rcs[] = "@(#) $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/manager/MM/assert.c,v 1.2 1997/06/29 15:44:29 night Exp $";


/*
 * $Log: assert.c,v $
 * Revision 1.2  1997/06/29 15:44:29  night
 * �����Ȥ��ѹ�
 *
 * �����������ɤ� SJIS ���� EUC ���ѹ�������
 * ��RCS �� Log �ޥ�������Υ����Ȥ��ɲä���(�ޤ�����Ƥ��ʤ��ä��ե�����Τ�)��
 *
 *
 */


#include "memory_manager.h"



/* ASSERT �ޥ����ƤӽФ��ؿ� */

/* __assert__ - ASSERT �ޥ���ˤ�äƸƤӽФ����ؿ�
 *
 * ��å���������Ϥ����ץ�����λ���롣
 *
 */ 
void
__assert__ (B *msg, B *file, W line)
{
  printf ("ASSERT: %s(%d)", file, line);
  printf ("%s\n", msg);
  for (;;)
    {
      slp_tsk ();
    }
}

