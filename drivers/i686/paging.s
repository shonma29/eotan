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

.globl fault_get_addr
.globl paging_set_directory
.globl paging_start
.globl tlb_flush


fault_get_addr:
	movl %cr2, %eax
	ret

paging_set_directory:
	movl 4(%esp), %eax
	movl %eax, %cr3
	ret

paging_start:
	movl %cr4, %eax
	/* set PSE, PGE */
	orb $0x90, %al
	movl %eax, %cr4

	movl %cr0, %eax
	/* set PG, AM, WP, NE, MP */
	orl $0x80050022, %eax
	/* clear CD, NW, EM */
	andl $0x9ffffffb, %eax
	movl %eax, %cr0

tlb_flush:
	movl %cr3, %eax
	movl %eax, %cr3
	ret
