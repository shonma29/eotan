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

.globl handle0
.globl handle1
.globl handle2
.globl handle3
.globl handle4
.globl handle5
.globl handle6
.globl handle7
.globl handle8
.globl handle9
.globl handle10
.globl handle11
.globl handle12
.globl handle13
.globl handle14
.globl handle15
.globl handle16
.globl handle17
.globl handle18
.globl handle19
.globl handle20
.globl handle21
.globl handle22
.globl handle23
.globl handle24
.globl handle25
.globl handle26
.globl handle27
.globl handle28
.globl handle29
.globl handle30
.globl handle31


/* division error (fault) */
handle0:
	pushl $0
	jmp abort_handler

/* debugger (fault/trap. see debug registers) */
handle1:
	pushl $1
	jmp abort_handler

/* nmi (interrupt) */
handle2:
	pushl $2
	jmp abort_handler

/* break point (trap) */
handle3:
	pushl $3
	jmp abort_handler

/* overflow (trap) */
handle4:
	pushl $4
	jmp abort_handler

/* out_of_bound (fault) */
handle5:
	pushl $5
	jmp abort_handler

/* invalid_operation_code (fault) */
handle6:
	pushl $6
	jmp abort_handler

/* no_coprocessor (fault) */
handle7:
	pushl $7
	jmp abort_handler

/* double_fault (abort with error code 0) */
handle8:
	pushl $8
	jmp abort_with_error_handler

/* coprocessor_segment_overrun (fault) */
handle9:
	pushl $9
	jmp abort_handler

/* invalid_tss (fault with error code) */
handle10:
	pushl $10
	jmp abort_with_error_handler

/* no_segment (fault with error code) */
handle11:
	pushl $11
	jmp abort_with_error_handler

/* stack_segment_fault (fault with error code) */
handle12:
	pushl $12
	pushl %ds
	pushl %es
	pushal
	movw $0x10, %ax
	movw %ax,%ds
	movw %ax,%es
	call protect_fault
	popal
	popl %es
	popl %ds
	addl $8, %esp
	iret

/* protection (fault with error code) */
handle13:
	pushl $13
	jmp abort_with_error_handler

/* page_fault (fault with special error code) */
handle14:
	pushl $14
	pushl %ds
	pushl %es
	pushal
	movw $0x10, %ax
	movw %ax,%ds
	movw %ax,%es
	call page_fault
	popal
	popl %es
	popl %ds
	addl $8, %esp
	iret

/* reserved */
handle15:
	pushl $15
	jmp abort_handler

/* math_fault (fault. see x87) */
handle16:
	pushl $16
	jmp abort_handler

/* alignment_check (fault with error code 0) */
handle17:
	pushl $17
	jmp abort_with_error_handler

/* machine_check (abort. see MSR) */
handle18:
	pushl $18
	jmp abort_handler

/* simd (fault) */
handle19:
	pushl $19
	jmp abort_handler

/* reserved */
handle20:
	pushl $20
	jmp abort_handler

/* reserved */
handle21:
	pushl $21
	jmp abort_handler

/* reserved */
handle22:
	pushl $22
	jmp abort_handler

/* reserved */
handle23:
	pushl $23
	jmp abort_handler

/* reserved */
handle24:
	pushl $24
	jmp abort_handler

/* reserved */
handle25:
	pushl $25
	jmp abort_handler

/* reserved */
handle26:
	pushl $26
	jmp abort_handler

/* reserved */
handle27:
	pushl $27
	jmp abort_handler

/* reserved */
handle28:
	pushl $28
	jmp abort_handler

/* reserved */
handle29:
	pushl $29
	jmp abort_handler

/* reserved */
handle30:
	pushl $30
	jmp abort_handler

/* reserved */
handle31:
	pushl $31
	jmp abort_handler

abort_handler:
	pushl %ds
	pushl %es
	pushal
	movw $0x10, %ax
	movw %ax,%ds
	movw %ax,%es
	call idt_abort
	popal
	popl %es
	popl %ds
	addl $4, %esp
	iret

abort_with_error_handler:
	pushl %ds
	pushl %es
	pushal
	movw $0x10, %ax
	movw %ax,%ds
	movw %ax,%es
	call idt_abort_with_error
	popal
	popl %es
	popl %ds
	addl $8, %esp
	iret
