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

.text

.set SELECTOR_KERN_DATA, 0x10
.set PIC_MASTER1, 0x0020

.globl handle32
.globl handle33


/* timer */
handle32:
	pushl $32
	jmp interrupt_handler

/* keyboard */
handle33:
	pushl $33
	jmp interrupt_handler

interrupt_handler:
	pushl %ds
	pushal
	movw $SELECTOR_KERN_DATA, %ax
	movw %ax,%ds
	call interrupt

	movl %eax, %edx
	movl 36(%esp), %eax
	addb $0x40, %al
	outb %al, $PIC_MASTER1

	testl %edx, %edx
	jnz end_interrupt
	call thread_switch

end_interrupt:
	popal
	popl %ds
	addl $4, %esp
	iret