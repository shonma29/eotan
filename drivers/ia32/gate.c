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

#include <string.h>
#include <core.h>
#include <func.h>
#include "gate.h"
#include "mpufunc.h"

static void gdt_set_segment(UH selector, UW base, UW limit, UB type, UB dpl);
static void gate_set(GateDescriptor *p,
		UH selector, W (*handler)(void), UB attr);


void idt_initialize(void)
{
	UW i;
	GateDescriptor desc;
	GateDescriptor *p = (GateDescriptor*)IDT_ADDR;

	desc.offsetLow = 0;
	desc.selector = kern_code;
	desc.copyCount = 0;
	desc.attr = (dpl_kern << 5) | interruptGate32;
	desc.offsetHigh = 0;

	for (i = 0; i <= MAX_IDT; i++)
		*p++ = desc;
}

void idt_set(UB no, UH selector, W (*handler)(void), UB type, UB dpl)
{
	GateDescriptor *p = (GateDescriptor*)IDT_ADDR;

	//TODO error check
	gate_set(&(p[no]), selector, handler, ATTR_PRESENT | (dpl << 5) | type);
}

void idt_abort(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		UW ecx, UW eax, UW es, UW ds, UW no,
		UW eip, UW cs, UW eflags)
{
	printk("abort(%d). task=%d\n"
		" cs=%x eip=%x eflags=%x\n"
		" ds=%x es=%x\n"
		" eax=%x ebx=%x ecx=%x edx=%x\n"
		" edi=%x esi=%x ebp=%x esp=%x\n",
			no, run_task->tskid, cs, eip, eflags, ds, es,
			eax, ebx, ecx, edx, edi, esi, ebp, esp);
	//TODO stop the thread
	halt();
	for (;;);
}

void idt_abort_with_error(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		UW ecx, UW eax, UW es, UW ds, UW no,
		UW err, UW eip, UW cs, UW eflags)
{
	printk("abort(%d). task=%d\n"
		" cs=%x eip=%x eflags=%x\n"
		" ds=%x es=%x error=%x\n"
		" eax=%x ebx=%x ecx=%x edx=%x\n"
		" edi=%x esi=%x ebp=%x esp=%x\n",
			no, run_task->tskid, cs, eip, eflags, ds, es,
			err, eax, ebx, ecx, edx, edi, esi, ebp, esp);
	//TODO stop the thread
	halt();
	for (;;);
}

static void gate_set(GateDescriptor *p,
		UH selector, W (*handler)(void), UB attr)
{
	//TODO error check
	p->offsetLow = ((UW)handler) & 0xffff;
	p->selector = selector;
	p->copyCount = 0;
	p->attr = attr;
	p->offsetHigh = (((UW)handler) >> 16) & 0xffff;
}

void gdt_initialize(void)
{
	SegmentDescriptor *p = (SegmentDescriptor*)GDT_ADDR;

	/* segments */
	memset(p, 0, sizeof(SegmentDescriptor));
	gdt_set_segment(kern_code, 0, 0xfffff, segmentCode, dpl_kern);
	gdt_set_segment(kern_data, 0, 0xfffff, segmentData, dpl_kern);
	gdt_set_segment(user_code, 0, 0x7fff0, segmentCode, dpl_user);
	gdt_set_segment(user_data, 0, 0x7fff0, segmentData, dpl_user);
	gdt_set_segment(user_stack, 0, 0x3ffff, segmentStack, dpl_user);

	/* call gate for service call */
	gate_set((GateDescriptor*)(&(p[call_service >> 3])),
			kern_code, syscall_handler,
			ATTR_PRESENT | (dpl_user << 5) | callGate32);
}

static void gdt_set_segment(UH selector, UW base, UW limit, UB type, UB dpl)
{
	SegmentDescriptor *p = (SegmentDescriptor*)GDT_ADDR;

	//TODO error check
	p = &(p[selector >> 3]);

	p->limitLow = limit & 0xffff;
	p->baseLow = base & 0xffff;
	p->baseMiddle = (base >> 16) & 0xff;
	p->type = ATTR_PRESENT | (dpl << 5) | type;
	p->limitHigh = ATTR_G_PAGE | ATTR_DB_32 | ((limit >> 16) & 0xf);
	p->baseHigh = (base >> 24) & 0xff;
}
