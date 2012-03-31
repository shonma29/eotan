/*	ITRON をトラップをかけて呼び出す(システムコール)
 *
 */

.globl	call_syscall
#if 0
		, call_posix
#endif
	
/* call_syscall: ITRON システムコールルーチン
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
call_syscall:
		pushl 	%ebp
		movl	%esp,%ebp
		movl	8(%ebp), %eax
		leal	12(%ebp), %edx
#if 0
		int	$64
#else
		lcall	$0x30,$0
#endif
		leave
		ret


#if 0
/* call_posix: POSIX システムコールルーチン(obsolute)
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
call_posix:
		pushl 	%ebp
		movl	%esp,%ebp
		movl	8(%ebp), %eax
		leal	12(%ebp), %edx
		int	$65
		leave	
		ret

#endif
