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
/**
 * Start kernel
 * @version 1.0
 * @since 1.0
 */
.text
.code16

.global _start

.set X86_64, 0

.set SELECTOR_CODE, 0x08
.set SELECTOR_DATA, 0x10
.set SELECTOR_STACK, 0x18

.set STACK_ADDR, 0x00008000
.set MEMORY_INFO_ADDR, 0x1000
.set PERIPHERAL_INFO_ADDR, 0x1ff0

.set VESA_INFO_ADDR, 0x1c00
.set VESA_MODE, 0x0118

.if X86_64
.set KTHREAD_DIR_ADDR, 0x00002000
.set MIN_MEMORY_MB, 8
.endif

/**
 * start on segment 0x0800.
 * @since 1.0
 */
_start:
	cli

	xorl %eax, %eax
	cpuid
.if X86_64
	cmpl $0, %eax
.else
	xorl %ebx, %ebx
	cmpl %ebx, %eax
.endif
	jle error_cpu
.if X86_64
	/* check x86_64 */
	movl $0x80000001, %eax
	cpuid
	movl $0x20000000, %eax
	andl %eax, %edx
	cmpl %eax, %edx
	jne error_cpu

	/* check LAHF */
	movl $0x00000001, %eax
	andl %eax, %ecx
	cmpl %eax, %ecx
	jne error_cpu

	/* check SSE3/SSSE3/CX16/SSE4_1/SSE4_2/POPCNT */
	cpuid
	movl $0x00982201, %eax
	andl %eax, %ecx
	cmpl %eax, %ecx
.else
	movl %ebx, %eax
	incb %al
	cpuid

	/* check SSE/FXSR/MMX/PGE/MTRR/SEP/APIC/CX8/MSR/PSE/FPU */
	movl $0x03803b29, %eax
	andl %eax, %edx
	cmpl %eax, %edx
.endif
	jne error_cpu

	/* get memory map */
	xorl %ebx, %ebx
	xorw %ax, %ax
	movw %ax, %es
	movw $MEMORY_INFO_ADDR + 4, %ax
	movw %ax, %di
memory_loop:
	movl $20, %ecx
	movl $0x534d4150, %edx
	movl $0x0000e820, %eax
	int $0x15
	jc error_memory

	addw $20, %di
	test %ebx, %ebx
	jnz memory_loop

	xorl %eax, %eax
	movw %di, %ax
	movl %eax, MEMORY_INFO_ADDR

	/* check PCI */
	movw $0xb101, %ax
	int $0x1a
	movw $PERIPHERAL_INFO_ADDR, %si
	movw %ax, (%si)

	/* check if supports VESA */
	xorw %ax, %ax
	movw %ax, %es
	movw $VESA_INFO_ADDR, %di
	movw $0x4f00, %ax
	int $0x10

	cmpw $0x004f, %ax
	jne error_vesa

	cmpw $0x0200, VESA_INFO_ADDR + 4
	jb error_vesa

	/* check if supports VGA 24bit color */
	movw $0x4f01, %ax
	movw $VESA_MODE, %cx
	int $0x10

	cmpw $0x004f, %ax
	jne error_vesa

	/* enter VESA */
	movw $0x4f02, %ax
	movw $VESA_MODE, %bx
	int $0x10

	cmpw $0x004f, %ax
	jne error_vesa

	/* enable A20 */
	call a20_wait

	movw $0x64, %dx
	movb $0xd1, %al
	outb %al, (%dx)
	call a20_wait

	movw $0x60, %dx
	movb $0xdf, %al
	outb %al, (%dx)
	call a20_wait

	movw $0x64, %dx
	movb $0xff, %al
	outb %al, (%dx)
	call a20_wait

	/* set GDT */
	lgdt gdt_ptr

	/* go to protect mode */
	movl %cr0, %eax
	orb $1, %al
	movl %eax, %cr0
	jmp flush_op

flush_op:
	movw $SELECTOR_DATA, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	lssl stack_ptr, %esp
.if X86_64
	ljmp $SELECTOR_CODE, $enter_32bit_segment

.code32
enter_32bit_segment:
	/* enable PAE */
	movl %cr4, %eax
	orl $0x20, %eax	/*TODO review setting */
	movl %eax, %cr4

	/* set page table */
	movl $KTHREAD_DIR_ADDR, %edx
	xorl %eax, %eax
	movl %edx, %edi
	movl $(4096 / 4 * 3), %ecx
	rep stosl

	leal (4096 + 7)(%edx), %edi
	movl %edi, (%edx)

	leal (8192 + 7)(%edx), %edi
	movl %edi, 4096(%edx)

	leal 8192(%edx), %edi
	movl $0x83, %eax
	movl $0x200000, %ebx
	movl $(MIN_MEMORY_MB / 2), %ecx

pte_lp:
	movl %eax, (%edi)
	addl %ebx, %eax
	addl $8, %edi
	loopne pte_lp

	movl %edx, %cr3

	/* go to long mode */
	movl $0xc0000080, %ecx	/*TODO review setting */
	rdmsr
	orl $0x100, %eax
	wrmsr

	/* start paging */
	movl %cr0, %eax
	orl $0x80000001, %eax	/*TODO review setting */
	movl %eax, %cr0

	/* enter 64bit segment */
	pushl $0x20
	pushl $_main
	lret
.else
	ljmp $SELECTOR_CODE, $_main
.endif
error_cpu:
	movw $message_cpu_error, %ax
	jmp die

error_memory:
	movw $message_memory_error, %ax
	jmp die

error_vesa:
	movw $message_vesa_error, %ax
	jmp die

/**
 * wait KBC
 * destroyed: al, dx
 * @since 1.0
 */
a20_wait:
	movw $0x64, %dx
	inb (%dx), %al
	andb $2, %al
	jnz a20_wait

	ret

/**
 * halt CPU
 * @since 1.0
 */
die:
	call puts
	cli
eternal:
	hlt
	jmp eternal

/**
 * Print string to console by BIOS.
 * destroyed: bx
 * @param ax address of string (ends with null)
 * @since 1.0
 */
puts:
	pushw %si
	movw %ax, %si

puts_inner:
	movw $0x000f, %bx
	movb $0x0e, %ah
	jmp puts_entry

puts_loop:
	int $0x10
	incw %si

puts_entry:
	movb (%si), %al
	andb %al, %al
	jne puts_loop

	popw %si
	ret


.data
.align 2
stack_ptr:
	.long STACK_ADDR
	.word SELECTOR_STACK

gdt_ptr:
.if X86_64
	.word 5 * 8 - 1
.else
	.word 4 * 8 - 1
.endif
	.long gdt_table

.align 8
gdt_table:
	/* 0x00: null */
	.byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00
	/* 0x08: kernel code */
	.byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00
	/* 0x10: kernel data */
	.byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00
	/* 0x18: kernel stack */
	.byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x96, 0xc0, 0x00
.if X86_64
	/* 0x20: 64bit code */
	.byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xaf, 0x00
.endif
message_cpu_error:
	.asciz "unsupported cpu"

message_memory_error:
	.asciz "cannot get memory map"

message_vesa_error:
	.asciz "cannot use VESA"

.org 256, 0
