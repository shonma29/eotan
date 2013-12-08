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
#include <mpu/desc.h>
#include <mpu/setting.h>
#include "gate.h"
#include "msr.h"
#include "mpufunc.h"
#include "tss.h"

static void idt_initialize(void);
static void gdt_initialize(void);
static void gdt_set_segment(const UH selector,
		const UW base, const UW limit,
		const UB type, const UB dpl, const UB option);


void mpu_initialize(void)
{
	gdt_initialize();
	idt_initialize();
}

static void idt_initialize(void)
{
	UW i;
	GateDescriptor desc;
	GateDescriptor *p = (GateDescriptor*)IDT_ADDR;

	desc.offsetLow = 0;
	desc.selector = kern_code;
	desc.copyCount = 0;
	desc.attr = (dpl_kern << 5) | interruptGate32;
	desc.offsetHigh = 0;

	for (i = 0; i < IDT_MAX_ENTRY; i++)
		*p++ = desc;

	idt_load();
}

static void gdt_initialize(void)
{
	SegmentDescriptor *p = (SegmentDescriptor*)GDT_ADDR;
	tss_t *tss = (tss_t*)kern_v2p((VP)TSS_ADDR);

	/* segments */
	memset(p, 0, GDT_MAX_ENTRY * sizeof(SegmentDescriptor));
	gdt_set_segment(kern_code, 0, 0xfffff,
			segmentCode, dpl_kern, ATTR_G_PAGE | ATTR_DB_32);
	gdt_set_segment(kern_data, 0, 0xfffff,
			segmentData, dpl_kern, ATTR_G_PAGE | ATTR_DB_32);
	gdt_set_segment(user_code, 0, 0xfffff,
			segmentCode, dpl_user, ATTR_G_PAGE | ATTR_DB_32);
	gdt_set_segment(user_data, 0, 0xfffff,
			segmentData, dpl_user, ATTR_G_PAGE | ATTR_DB_32);

	memset(tss, 0, sizeof(*tss));
	tss->ss0 = kern_data;
	gdt_set_segment(dummy_tss, (UW)tss, sizeof(*tss),
			segmentTss, dpl_kern, 0);

	gdt_load();

	msr_write(sysenter_cs_msr, kern_code);
}

static void gdt_set_segment(const UH selector,
		const UW base, const UW limit,
		const UB type, const UB dpl, const UB option)
{
	SegmentDescriptor *p = (SegmentDescriptor*)GDT_ADDR;

	//TODO error check
	p = &(p[selector >> 3]);

	p->limitLow = limit & 0xffff;
	p->baseLow = base & 0xffff;
	p->baseMiddle = (base >> 16) & 0xff;
	p->type = ATTR_PRESENT | (dpl << 5) | type;
	p->limitHigh = option | ((limit >> 16) & 0xf);
	p->baseHigh = (base >> 24) & 0xff;
}
