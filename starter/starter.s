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

.set SELECTOR_CODE, 0x08
.set SELECTOR_DATA, 0x10
.set SELECTOR_STACK, 0x18

.set STACK_ADDR, 0x00008000
.set MEMORY_INFO_ADDR, 0x3000
.set VESA_INFO_ADDR, 0x3c00

/**
 * start on segment 0x0800.
 * @since 1.0
 */
_start:
	cli

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
	movw $0x0112, %cx
	int $0x10

	cmpw $0x004f, %ax
	jne error_vesa

	/* enter VESA */
/*
	movw $0x4f02, %ax
	movw $0x0112, %bx
	int $0x10

	cmpw $0x004f, %ax
	jne error_vesa
*/
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
	lssl stack_ptr, %esp
	ljmp $SELECTOR_CODE, $_main

error_vesa:
	movw $message_vesa_error, %ax
	jmp die

error_memory:
	movw $message_memory_error, %ax
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
	.word 4 * 8 - 1
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

message_vesa_error:
	.asciz "cannot use VESA"

message_memory_error:
	.asciz "cannot get memory map"

.org 256, 0
