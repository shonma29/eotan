/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

.globl fiber_start
.globl fiber_switch

.text

fiber_start:
	movl 4(%esp), %eax
	movl 8(%esp), %ecx
	subl $20, %eax
	movl $fiber_end, %edx
	movl %ecx, (%eax)
	movl %edx, 4(%eax)
	movl 12(%esp), %ecx
	movl 16(%esp), %edx
	movl %ecx, 8(%eax)
	movl %edx, 12(%eax)
	movl 20(%esp), %ecx
	movl %ecx, 16(%eax)
	pushl %edi
	pushl %esi
	pushl %ebp
	pushl %ebx
	movl %esp, (%edx)
	movl %eax, %esp
	ret

fiber_end:
	movl 4(%esp), %edx
	movl $1, %eax
	jmp resume

fiber_switch:
	movl 4(%esp), %ecx
	movl 8(%esp), %edx
	pushl %edi
	pushl %esi
	pushl %ebp
	pushl %ebx
	movl %esp, (%ecx)
	xorl %eax, %eax
resume:
	movl (%edx), %esp
	popl %ebx
	popl %ebp
	popl %esi
	popl %edi
	ret
