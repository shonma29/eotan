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
.set PIC_SLAVE1, 0x00a0

.globl handle32
.globl handle33
.globl handle44
.globl handle46
.globl handle47


/* primary ata */
handle46:
	pushl $46
	jmp interrupt_slave

/* secondary ata */
handle47:
	pushl $47
	jmp interrupt_slave

/* psaux */
handle44:
	pushl $44

interrupt_slave:
	pushl %ds
	pushal
	movw $SELECTOR_KERN_DATA, %ax
	movw %ax,%ds
	call interrupt
	movb $0x20, %al
	outb %al, $PIC_SLAVE1
	jmp ret_interrupt

/* keyboard */
handle33:
	pushl $33
	jmp interrupt_master

/* timer */
handle32:
	pushl $32

interrupt_master:
	pushl %ds
	pushal
	movw $SELECTOR_KERN_DATA, %ax
	movw %ax,%ds
	call interrupt
	movb $0x20, %al

ret_interrupt:
	outb %al, $PIC_MASTER1
	popal
	popl %ds
	addl $4, %esp
	iret
