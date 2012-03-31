/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/fork.c,v 1.7 2000/04/03 14:33:04 naniwa Exp $ */
static char rcsid[] =
    "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/fork.c,v 1.7 2000/04/03 14:33:04 naniwa Exp $";

/*
 * $Log: fork.c,v $
 * Revision 1.7  2000/04/03 14:33:04  naniwa
 * to call timer handler in task
 *
 * Revision 1.6  1999/11/10 10:47:02  naniwa
 * minor fix
 *
 * Revision 1.5  1999/07/30 08:25:34  naniwa
 * implemented
 *
 * Revision 1.4  1999/03/21 00:57:28  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.3  1998/02/25 12:51:34  night
 * �ץ�����θ�ͭ�ǡ�������򡢤��줾������ѿ��Ǹ���Ū�ˤ�ĤΤǤϤʤ���
 * lowlib_data �Ȥ����ݥ��󥿤Ǵ�������褦�ˤ�����
 * �����ѹ���ȼ�����ץ�����˰�ä����������/���Ȥ�������ϡ��ݥ���
 * lowlib_data ��𤷤ƹԤ��褦���ѹ�������
 *
 * lowlib_data ���ͤϡ��ޥ��� LOWLIB_DATA �ǻ��ꤷ�����ɥ쥹 (0x7fff0000)
 * �����ꤵ��롣���Υ��ɥ쥹�ϡ��ץ�����˰�ä�ʪ���ڡ����˥ޥåԥ�
 * ����롣���ۥ��ɥ쥹�ϸ���������ºݤ��ΰ�ϥץ�����˰�ä�����¸
 * �ߤ��Ƥ��롣LOWLIB ����������ץ�����ξ���Ϥ����ΰ�������褦��
 * ���롣
 *
 * Revision 1.2  1997/12/17 14:15:02  night
 * psys_fork(fork �����ƥॳ����ν����ؿ�) ����Ȥ��ä���
 *
 * Revision 1.1  1997/08/31 13:10:44  night
 * �ǽ����Ͽ
 *
 *
 *
 */


#include "../lowlib.h"

extern int bzero(VP, int);

/* fork �����ƥॳ����ν���
 *
 */
int psys_fork(void *argp)
{
    ER error;
    struct posix_request req;
    struct posix_response res;
    ID child_main;
    ID child_signal;
    ID myself;
    T_CTSK task_info;
    struct arg {
	int esp; int ebp;
        int ebx; int ecx; int edx; int esi; int edi;
        VP fork_entry;
    } *args;

    args = (struct arg *) argp;

    /* �ҥץ����Υ���������
     * �¹Գ��ϥ��ɥ쥹�ϡ�fork_entry �����ꤷ�Ƥ�����
     *
     */

    /* �ҥץ����� main task ������ */
    bzero(&task_info, sizeof(task_info));
    task_info.tskatr = TA_HLNG;
    task_info.startaddr = args->fork_entry;
    task_info.itskpri = POSIX_TASK_LEVEL;
    task_info.stksz = POSIX_STACK_SIZE;
    error = vcre_tsk(&task_info, &child_main);
    if (error != E_OK)
	return (error);

    /* �ҥץ����� signal task ������ */
    /* �������Ǥ��������ʤ� */
    child_signal = 0;

    /* �ҥץ����� UNIQ PAGE �μ��� */

    /* POSIX manager �θƤӽФ� 

     * ���������ꤷ�ơ�POSIX manager �˥�å����������롣
     */
    req.param.par_fork.main_task = child_main;
    req.param.par_fork.signal_task = child_signal;

    error = _make_connection(PSC_FORK, &req, &res);
    if (error != E_OK) {
	/* What should I do? */
	del_tsk(child_main);
	ERRNO = error;
	return (-1);
    } else if (res.errno) {
	del_tsk(child_main);
	ERRNO = res.errno;
	return (-1);
    }

    /* stack �Υ��ԡ� */
    get_tid(&myself);
    vcpy_stk(myself, args->esp, args->ebp,
	     args->ebx, args->ecx, args->edx, args->esi, args->edi,
	     child_main);

    /* FPU ��ͭ���ˤ��� */
    vuse_fpu(myself);

    /* �ҥץ�����ͭ���ˤ��� */
    sta_tsk(child_main, 0);
    return (res.status);
}
