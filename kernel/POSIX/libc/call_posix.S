/*	ITRON にトラップをかけて呼び出す(システムコール)
 *
 */

/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/call_posix.s,v 1.1 1997/08/31 13:18:32 night Exp $ */
	


.globl	call_lowlib
	


/* call_posix: POSIX システムコールルーチン
 *
 * . スタック形式:
 *
 *		+--------------------+
 *		|    return addr     |	+4
 *		+--------------------+
 *		|    syscall no.     |  +8
 *		+--------------------+
 *		|    argument #1     |  +12
 *		+--------------------+
 *		|    argument #2     |  +16
 *		+--------------------+
 *		|    argument #3     |
 *
 */
call_lowlib:
		pushl 	%ebp
		movl	%esp,%ebp
		movl	8(%ebp), %eax
		leal	12(%ebp), %edx
#if 0
		int	$65
#else
		lcall	$0x38,$0
#endif
	        movl 2147418144,%edx
		negl %edx
	        movl %edx,errno
		leave
		ret


