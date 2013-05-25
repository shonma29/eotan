/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
    $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/fork.S,v 1.2 1999/11/10 10:39:31 naniwa Exp $
*/

.text
.globl fork

fork:
	pushl	%edi
	pushl	%esi
	pushl	%ebp
	pushl	%ebx
	movl	%esp,%eax
	pushl	%eax
	call	_fork
	addl	$20,%esp
	ret
