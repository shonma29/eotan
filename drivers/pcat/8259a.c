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
#include <core.h>
#include <mpufunc.h>
#include <archfunc.h>
#include <mpu/io.h>
#include <arch/8259a.h>
#include <nerve/sync.h>
#include "handler.h"

static UH ports[] = {
	pic_master2, pic_slave2
};


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

	pic_mask_all();

	idt_set(PIC_IR_VECTOR(ir_pit), handle32);
	idt_set(PIC_IR_VECTOR(ir_keyboard), handle33);
	idt_set(PIC_IR_VECTOR(ir_mouse), handle44);
	idt_set(PIC_IR_VECTOR(ir_ide_primary), handle46);
	idt_set(PIC_IR_VECTOR(ir_ide_secondary), handle47);
}

ER pic_reset_mask(const UB ir)
{
	UH port;
	UB mask;

	if (ir >= 16)
		return E_PAR;

	port = ports[ir >> 3];
	mask = ~PIC_IR_BIT(ir & 7);

	enter_critical();
	outb(port, mask & inb(port));
	leave_critical();

	return E_OK;
}

