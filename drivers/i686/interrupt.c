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
#include <interrupt.h>
#include <mpu/desc.h>
#include <nerve/kcall.h>
#include <nerve/func.h>
#include <nerve/sync.h>
#include "handler.h"
#include "interrupt.h"

typedef struct _service {
	void (*isr)(VP_INT);
	VP_INT exinf;
	struct _service *next;
	INTNO interrupt_no;
	ID service_id;
} service_t;

// entries MUST be larger than service_map
static service_t entries[MAX_INTERRUPT_SERVICE];
static service_t *unmapped;
static service_t *service_map[IDT_MAX_ENTRY];

static void dummy(VP_INT);
static service_t *get_unmapped_entry(void);
static _Noreturn void fault(UW, UW, UW, UW, UW, UW, UW, UW, UW, UW, UW, UW, UW);
static _Noreturn void fault_with_error(UW, UW, UW, UW, UW, UW, UW, UW, UW, UW,
		UW, UW, UW, UW);


static void dummy(VP_INT exinf)
{
}

//TODO test
static service_t *get_unmapped_entry(void)
{
	service_t *entry = unmapped;
	if (entry)
		unmapped = entry->next;

	return entry;
}

//TODO test
ER interrupt_initialize(void)
{
	kcall->printk("interrupt_initialize\n");

	unsigned int i;
	// MPU exceptions
	for (i = 0; i < sizeof(handlers) / sizeof(handlers[0]); i++) {
		service_t *entry = &(entries[i]);
		entry->isr = dummy;
		entry->next = NULL;
		entry->interrupt_no = i;
		entry->service_id = i;
		service_map[i] = entry;

		idt_set(i, handlers[i]);
	}

	// hardware interruptions
	for (; i < sizeof(service_map) / sizeof(service_map[0]); i++) {
		service_t *entry = &(entries[i]);
		entry->isr = dummy;
		entry->next = NULL;
		entry->interrupt_no = i;
		entry->service_id = i;
		service_map[i] = entry;
	}

	// unmapped entry list
	unmapped = &entries[i];
	for (; i < sizeof(entries) / sizeof(entries[0]) - 1; i++) {
		entries[i].isr = dummy;
		entries[i].next = &(entries[i + 1]);
		entries[i].service_id = i;
	}

	entries[i].isr = dummy;
	entries[i].next = NULL;
	entries[i].service_id = i;
	return E_OK;
}

//TODO test
ER_ID interrupt_bind(T_CISR *pk_cisr)
{
	if (pk_cisr->isratr != TA_HLNG)
		return E_RSATR;

	if (sizeof(service_map) / sizeof(service_map[0]) <= pk_cisr->intno)
		return E_ID;

	if (pk_cisr->isr == NULL)
		return E_PAR;

	for (service_t *p = service_map[pk_cisr->intno]; p; p = p->next)
		if (p->isr == dummy) {
			p->isr = pk_cisr->isr;
			p->exinf = pk_cisr->exinf;
			kcall->printk("interrupt_bind[%d] 0x%x\n",
					pk_cisr->intno, pk_cisr->isr);
			return p->service_id;
		} else if (p->next == NULL) {
			service_t *entry = get_unmapped_entry();
			if (!entry)
				return E_NOID;

			entry->isr = pk_cisr->isr;
			entry->exinf = pk_cisr->exinf;
			entry->next = NULL;
			entry->interrupt_no = pk_cisr->intno;
			p->next = entry;
			kcall->printk("interrupt_bind[%d] 0x%x\n",
					pk_cisr->intno, pk_cisr->isr);
			return entry->service_id;
		}

	return E_ID;
}

//TODO test
ER interrupt_unbind(ID service_id)
{
	if ((service_id < 0)
			|| (sizeof(entries) / sizeof(entries[0]) <= service_id))
		return E_ID;

	service_t *entry = &(entries[service_id]);
	if (entry->isr == dummy)
		return E_OBJ;

	service_t *prev = NULL;
	for (service_t *p = service_map[entry->interrupt_no]; p;
			prev = p, p = p->next)
		if (p == entry) {
			if (prev)
				prev->next = entry->next;
			else if (entry->next)
				service_map[entry->interrupt_no] = entry->next;
			else {
				entry->isr = dummy;
				return E_OK;
			}

			entry->isr = dummy;
			entry->next = unmapped;
			unmapped = entry;
			return E_OK;
		}

	return E_ID;
}

//TODO test
void interrupt(const UW edi, const UW esi, const UW ebp, const UW esp,
		const UW ebx, const UW edx, const UW ecx, const UW eax,
		const UW ds, const UW no, const UW eip,
		const UW cs, const W eflags)
{
	sysinfo->sync.state.interrupt_nest++;
	service_t *p = service_map[no];
	do {
		if (p->isr == dummy)
			fault(edi, esi, ebp, esp, ebx, edx,
					ecx, eax, ds, no, eip, cs, eflags);

		p->isr(p->exinf);
	} while ((p = p->next));

	sysinfo->sync.state.interrupt_nest--;
	if (!(sysinfo->sync.dispatch_skippable))
		kcall->dispatch();
}

//TODO test
void interrupt_with_error(const UW edi, const UW esi, const UW ebp,
		const UW esp,
		const UW ebx, const UW edx, const UW ecx, const UW eax,
		const UW ds, const UW no, const UW err, const UW eip,
		const UW cs, const W eflags)
{
	sysinfo->sync.state.interrupt_nest++;
	//TODO for debug
	kcall->printk("interrupt(%d). thread=%d\n"
		" cs=%x eip=%x eflags=%x ds=%x error=%x\n"
		" eax=%x ebx=%x ecx=%x edx=%x\n"
		" edi=%x esi=%x ebp=%x esp=%x\n",
			no, kcall->thread_get_id(),
			cs, eip, eflags, ds,
			err, eax, ebx, ecx, edx, edi, esi, ebp, esp);

	service_t *p = service_map[no];
	do {
		if (p->isr == dummy)
			fault_with_error(edi, esi, ebp, esp, ebx, edx,
					ecx, eax, ds, no, err, eip, cs, eflags);

		p->isr(p->exinf);
	} while ((p = p->next));

	sysinfo->sync.state.interrupt_nest--;
	if (!(sysinfo->sync.dispatch_skippable))
		kcall->dispatch();
}

static _Noreturn void fault(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		UW ecx, UW eax, UW ds, UW no,
		UW eip, UW cs, UW eflags)
{
	kcall->printk("abort(%d). thread=%d\n"
		" cs=%x eip=%x eflags=%x ds=%x\n"
		" eax=%x ebx=%x ecx=%x edx=%x\n"
		" edi=%x esi=%x ebp=%x esp=%x\n",
			no, kcall->thread_get_id(),
			cs, eip, eflags, ds,
			eax, ebx, ecx, edx, edi, esi, ebp, esp);
	panic("fault");
}

static _Noreturn void fault_with_error(
		UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		UW ecx, UW eax, UW ds, UW no,
		UW err, UW eip, UW cs, UW eflags)
{
	kcall->printk("abort(%d). thread=%d\n"
		" cs=%x eip=%x eflags=%x ds=%x error=%x\n"
		" eax=%x ebx=%x ecx=%x edx=%x\n"
		" edi=%x esi=%x ebp=%x esp=%x\n",
			no, kcall->thread_get_id(),
			cs, eip, eflags, ds,
			err, eax, ebx, ecx, edx, edi, esi, ebp, esp);
	panic("fault_with_error");
}
