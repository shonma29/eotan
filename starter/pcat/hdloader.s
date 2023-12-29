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
 * Boot Loader for hard disk.
 * @version 1.0
 * @since 1.0
 */
.text
.code16

.global _start

.include "starter/arch/loader.inc"

.set BIOS_MEMORY_KB, 32

.set BLOCK_NUM, 64


/**
 * start
 * @since 1.0
 */
_start:
	xorw %ax, %ax
	movw %ax, %ds
	movw %ax, %ss
	movw $0x7c00, %ax
	movw %ax, %sp

	/* print message */
	movw $message_product, %ax
	call putsn

	/* get free memory */
	int $0x12
	movw %ax, %cx
	call putd

	movw $message_memory, %ax
	call putsn

	/* initialize */
	movw $STARTER_SECTORS, %di
	movw $dap, %si

	/* check image size */
	subw $BIOS_MEMORY_KB, %cx
	addw %cx, %cx
	cmpw %di, %cx
	jnc check_extension

	movw $message_too_large, %ax
	call putsn
	jmp die

check_extension:
	/* check extensions present */
	movb $0x41, %ah
	movw $0x55aa, %bx
	movb $0x80, %dl
	int $0x13
	jc not_support_LBA

	cmpw $0xaa55, %bx
	jne not_support_LBA

	andb $1, %cx
	jne read_entry

not_support_LBA:
	movw $message_not_support_LBA, %ax
	call putsn
	jmp die

read_lp:
	/* check left sectors */
	movw $BLOCK_NUM, %ax
	movw %di, %bx
	cmpw %bx, %ax
	jnc read_body
	movw %ax, %bx

read_body:
	subw %bx, %di

	/* read */
	movw %bx, 2(%si)
	movb $0x80, %dl
	movw $0x55aa, %bx
	movb $0x42, %ah
	int $0x13
	jc read_error

	/* put dot */
	movw $0x000f, %bx
	movw $0x0e2e, %ax
	int $0x10

	/* add segment */
	addw $(BLOCK_NUM * 512 / 16), 6(%si)
	addw $BLOCK_NUM, 8(%si)

read_entry:
	testw %di, %di
	jnz read_lp

done:
	movw $eol, %ax
	call puts
	jmp STARTER_ADDRESS

read_error:
	call puth
	movw $message_error, %ax
	call putsn

die:
	cli

eternal:
	hlt
	jmp eternal


/**
 * Print string and EOL to console by BIOS.
 * destroyed: bx
 * @param ax address of string (ends with null)
 * @since 1.0
 */
putsn:
	call puts
	movw $eol, %ax
	call puts
	ret

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
 
/**
 * Print decimal to console by BIOS.
 * destroyed: ax, bx, dx
 * @param ax number
 * @since 1.0
 */
putd:
	pushw %si
	movw $10, %bx
	movw $dec_eos, %si

todec_loop:
	xorw %dx, %dx
	divw %bx

	decw %si
	addb $0x30, %dl
	movb %dl, (%si)

	andw %ax, %ax
	jne todec_loop

	jmp puts_inner

/**
 * Print hex to console by BIOS.
 * destroyed: ax, bx, cx, dx
 * @param ah number
 * @since 1.0
 */
puth:
	pushw %si
	xorb %dh, %dh
	movb %ah, %dl
	movw $hex_table, %si
	movb $0x0e, %ah
	movw $0x000f, %cx

	movw %dx, %bx
	shrb $4, %bl
	movb (%bx, %si), %al
	movw %cx, %bx
	int $0x10

	movw %dx, %bx
	andb $0xf, %bl
	movb (%bx, %si), %al
	movw %cx, %bx
	int $0x10

	popw %si
	ret

.align 2

/**
 * message about product
 * @since 1.0
 */
message_product:
	.asciz "hdloader 1.0"
eol:
	.asciz "\r\n"

/**
 * message about memory
 * @since 1.0
 */
message_memory:
	.asciz "KB free memory"

/**
 * message too large image
 * @since 1.0
 */
message_too_large:
	.asciz "too large image"

/**
 * message not support LBA
 * @since 1.0
 */
message_not_support_LBA:
	.asciz "not support LBA"

/**
 * message when error
 * @since 1.0
 */
message_error:
	.asciz " error"

/**
 * hex string table
 * @since 1.0
 */
hex_table:
	.ascii "0123456789abcdef"

.align 2

/**
 * decimal string buffer
 * @since 1.0
 */
dec:
	.ascii "00000"
dec_eos:
	.byte 0

.align 2

/**
 * Disk Address Packet
 * @since 1.0
 */
dap:
	.byte 0x10
	.byte 0
sectors_to_read:
	.word 0
offset_to_store:
	.word 0
segment_to_store:
	.word STARTER_SEGMENT
position_to_read:
	.word 1
	.word 0
	.word 0
	.word 0

/**
 * padding
 * @since 1.0
 */
.org 510, 0

/**
 * signature
 * @since 1.0
 */
.word 0xaa55
