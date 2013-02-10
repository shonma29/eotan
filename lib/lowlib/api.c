/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/api.c,v 1.6 2000/06/01 08:44:40 naniwa Exp $ */

/*
 * $Log: api.c,v $
 * Revision 1.6  2000/06/01 08:44:40  naniwa
 * to implement getdents
 *
 * Revision 1.5  2000/02/16 08:17:04  naniwa
 * to reorder POSIX system calls
 *
 * Revision 1.4  1999/03/21 00:59:46  monaka
 * Function watipid was renamed to waitpid.
 *
 * Revision 1.3  1999/02/24 13:13:12  night
 * Changed function name for sigsuspend.
 *
 * Revision 1.2  1997/08/31 13:13:42  night
 * とりあえず、OS ファイルに入れるところまでできた。
 *
 * Revision 1.1  1996/11/11  13:36:05  night
 * IBM PC 版への最初の登録
 *
 * ----------------
 *
 *
 */

/*
 *	POSIX Envinronment systemcall table.
 */

#include "lowlib.h"


#define DEF_SYSCALL(x)		{ (int (*)())psys_ ## x, #x }

static int psys_noaction(void *argp);

struct syscall_entry syscalls[] =
{
  DEF_SYSCALL (noaction),	/*  0 */
  DEF_SYSCALL (access),		/*  1 */
  DEF_SYSCALL (noaction),		/*  2 */
  DEF_SYSCALL (noaction),		/*  3 */
  DEF_SYSCALL (noaction),		/*  4 */
  DEF_SYSCALL (noaction),		/*  5 */
  DEF_SYSCALL (dup),		/*  6 */
  DEF_SYSCALL (exec),		/*  7 */
  DEF_SYSCALL (exit),		/*  8 */
  DEF_SYSCALL (fcntl),		/*  9 */
  DEF_SYSCALL (fork),		/* 10 */
  DEF_SYSCALL (noaction),	/* 11 */ /* fstat */
  DEF_SYSCALL (noaction),	/* 12 */
  DEF_SYSCALL (noaction),	/* 13 */
  DEF_SYSCALL (noaction),		/* 14 */
  DEF_SYSCALL (noaction),		/* 15 */
  DEF_SYSCALL (noaction),	/* 16 */
  DEF_SYSCALL (noaction),		/* 17 */
  DEF_SYSCALL (noaction),		/* 18 */
  DEF_SYSCALL (noaction),		/* 19 */
  DEF_SYSCALL (noaction),		/* 20 */
  DEF_SYSCALL (noaction),		/* 21 */
  DEF_SYSCALL (noaction),		/* 22 */
  DEF_SYSCALL (noaction),		/* 23 */
  DEF_SYSCALL (noaction),		/* 24 */
  DEF_SYSCALL (setpgid),	/* 25 */
  DEF_SYSCALL (noaction),		/* 26 */
  DEF_SYSCALL (noaction),		/* 27 */
  DEF_SYSCALL (umask),		/* 28 */
  DEF_SYSCALL (noaction),		/* 29 */
  DEF_SYSCALL (utime),		/* 30 */
  DEF_SYSCALL (waitpid),	/* 31 */
  DEF_SYSCALL (noaction),		/* 32 */
  DEF_SYSCALL (brk),		/* 33 */ /* BRK */
  DEF_SYSCALL (noaction),	/* 34 */
  DEF_SYSCALL (mount),		/* 35 */
  DEF_SYSCALL (noaction),	/* 36 */ /* mountroot */
  DEF_SYSCALL (noaction),		/* 37 */
  DEF_SYSCALL (umount),		/* 38 */
  DEF_SYSCALL (kill),		/* 39 */
  DEF_SYSCALL (misc),		/* 40 */
  DEF_SYSCALL (dup2),		/* 41 */
  DEF_SYSCALL (sleep),	/* 42 */
};


int	nsyscall = (sizeof (syscalls) / sizeof (syscalls[0]));

static int psys_noaction(void *argp)
{
	errno = ENOSYS;

	return NULL;
}
