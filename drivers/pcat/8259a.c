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
#include <mpu/io.h>
#include "8259a.h"


void pic_initialize(void)
{
	/* initialize master */
	outb(pic_master1, PIC_ICW1_BASE | PIC_ICW1_NEED_IC4);
	outb(pic_master2, PIC_MASTER_VECTOR & PIC_VECTOR_MASK);
	outb(pic_master2, 1 << PIC_CASCADE_NO);
	outb(pic_master2, PIC_ICW4_8086);

	/* initialize slave */
	outb(pic_slave1, PIC_ICW1_BASE | PIC_ICW1_NEED_IC4);
	outb(pic_slave2, PIC_SLAVE_VECTOR & PIC_VECTOR_MASK);
	outb(pic_slave2, PIC_CASCADE_NO);
	outb(pic_slave2, PIC_ICW4_8086);

	/* set defaut mask */
	outb(pic_master2, ~PIC_IR_BIT(ir_cascade & PIC_INT_NO_MASK));
	outb(pic_slave2, ~0);
}
