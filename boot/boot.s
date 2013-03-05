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
 * Boot Loader for floppy disk.
 * @version 1.0
 * @since 1.0
 */
.text
.code16

.global _start

.include "starter.inc"

.set IPL_SEGMENT, 0x07c0
.set BOOT_SEGMENT, 0x0800
.set BOOT_ADDRESS, 0x8000
.set BIOS_MEMORY_KB, 32

.set TRACK_NUM, 18
.set DMA_LIMIT_SECTOR, 64


/**
 * start
 * @since 1.0
 */
_start:
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
	movw $BOOT_SEGMENT, %ax
	movw %ax, %es
	movw $left_sectors, %di

	/* check image size */
	subw $BIOS_MEMORY_KB, %cx
	movw (%di), %ax
	addw %cx, %cx
	cmpw %ax, %cx
	jnc read_entry

	movw $message_too_large, %ax
	call putsn
	jmp die
read_lp:
	/* check track_offset */
	movw $TRACK_NUM, %ax
	subw 2(%di), %ax
	cmpw %bx, %ax
	jnc in_track1

	movw %ax, %bx

in_track1:
	/* check dma_offset */
	movw $DMA_LIMIT_SECTOR, %ax
	subw 4(%di), %ax
	cmpw %bx, %ax
	jnc in_dma1

	movw %ax, %bx

in_dma1:
	movw %bx, %si

	/* read */
	xorb %dl, %dl
	movw 6(%di), %cx
	movb %bl, %al
	movb 8(%di), %dh
	xorw %bx, %bx
	movb $0x02, %ah
	int $0x13
	jc read_error

	/* put dot */
	movw $0x000f, %bx
	movw $0x0e2e, %ax
	int $0x10

	/* subtract left_sectors */
	movw %si, %bx
	subw %bx, (%di)
	addb %bl, 6(%di)

	/* add track_offset */
	movw 2(%di), %ax
	addw %bx, %ax
	cmpw $TRACK_NUM, %ax
	jne in_track2

	/* reset sector */
	movb $1, %ah
	movb %ah, 6(%di)

	/* add cylinder */
	movb 8(%di), %al
	addb %al, 7(%di)

	/* invert head */
	incb %al
	andb %ah, %al
	movb %al, 8(%di)

	xorw %ax, %ax

in_track2:
	movw %ax, 2(%di)

	/* add dma_offset */
	movw 4(%di), %ax
	addw %bx, %ax
	cmpw $DMA_LIMIT_SECTOR, %ax
	jne in_dma2

	xorw %ax, %ax

in_dma2:
	movw %ax, 4(%di)

	/* add segment */
	movw %es, %ax
	shlw $5, %bx
	addw %bx, %ax
	movw %ax, %es

read_entry:
	/* get left_sectors */
	movw (%di), %bx
	testw %bx, %bx
	jnz read_lp

done:
	call stop_motor
	movw $eol, %ax
	call puts
	jmp BOOT_ADDRESS

read_error:
	call puth
	movw $message_error, %ax
	call putsn

die:
	call stop_motor
	cli

eternal:
	hlt
	jmp eternal


/**
 * Stop FDD moter
 * destroyed: al, dx
 * @since 1.0
 */
stop_motor:
	movw $0x03f2, %dx
	movb $0x0c, %al
	outb %al, (%dx)
	ret

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
	.asciz "fdboot 1.0"
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
 * disk parameters
 * @since 1.0
 */
left_sectors:
	.word STARTUP_SECTORS
track_offset:
	.word 1
dma_offset:
	.word 0
sector:
	.byte 2
cylinder:
	.byte 0
head:
	.byte 0

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
