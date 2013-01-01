/*	ITRON をトラップをかけて呼び出す(システムコール)
 *
 */

.globl	call_syscall
	
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
		movl	4(%esp), %eax
		leal	8(%esp), %edx
		lcall	$0x30,$0
		ret
