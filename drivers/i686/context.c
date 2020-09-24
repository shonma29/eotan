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
#include <mpu/setting.h>
#include <nerve/config.h>
#include <func.h>
#include "context.h"
#include "eflags.h"
#include "gate.h"
#include "mpufunc.h"
#include "msr.h"
#include "tss.h"

static VP current_page_table = NULL;
static thread_t *fpu_holder = NULL;

static void api_set_kernel_sp(const VP);
static VP_INT *context_create_kernel(VP_INT *, const UW, const FP);
static VP_INT *context_create_user(VP_INT *, const UW, const FP, const VP);


void context_initialize(void)
{
	//TODO move to starter
	paging_reset();
}

static void api_set_kernel_sp(const VP addr)
{
	tss_t *tss = (tss_t *) TSS_ADDR;

	tss->esp0 = (uintptr_t) addr;
	msr_write(sysenter_esp_msr, (uintptr_t) addr);
}

static VP_INT *context_create_kernel(VP_INT *sp, const UW eflags, const FP eip)
{
	VP_INT *esp0;

	/* eflags */
	*--sp = eflags;
	/* cs */
	*--sp = kern_code;
	/* eip */
	*--sp = (VP_INT) eip;
	/* ds */
	*--sp = kern_data;
	/* es */
	*--sp = kern_data;
	/* fs */
	*--sp = kern_data;
	/* gs */
	*--sp = kern_data;
	esp0 = sp;

	/* eax */
	*--sp = 0;
	/* ecx */
	*--sp = 0;
	/* edx */
	*--sp = 0;
	/* ebx */
	*--sp = 0;
	/* kernel esp */
	*--sp = (VP_INT) esp0;
	/* ebp */
	*--sp = 0;
	/* esi */
	*--sp = 0;
	/* edi */
	*--sp = 0;

	return sp;
}

static VP_INT *context_create_user(VP_INT *sp, const UW eflags, const FP eip,
		const VP esp)
{
	VP_INT *esp0;

	/* ss */
	*--sp = user_data | dpl_user;
	/* user esp */
	*--sp = (VP_INT) esp;
	/* eflags */
	*--sp = eflags;
	/* cs */
	*--sp = user_code | dpl_user;
	/* eip */
	*--sp = (VP_INT) eip;
	/* ds */
	*--sp = user_data;
	/* es */
	*--sp = user_data;
	/* fs */
	*--sp = user_data;
	/* gs */
	*--sp = user_data;
	esp0 = sp;

	/* eax */
	*--sp = 0;
	/* ecx */
	*--sp = 0;
	/* edx */
	*--sp = 0;
	/* ebx */
	*--sp = 0;
	/* kernel esp */
	*--sp = (VP_INT) esp0;
	/* ebp */
	*--sp = 0;
	/* esi */
	*--sp = 0;
	/* edi */
	*--sp = 0;

	return sp;
}

void context_release(const thread_t *th)
{
	if (th == fpu_holder)
		fpu_holder = NULL;
}

void context_switch(thread_t *prev, thread_t *next)
{
	if (!is_kthread(next)) {
		if (next->mpu.cr3 != current_page_table) {
			paging_set_directory(next->mpu.cr3);
			current_page_table = next->mpu.cr3;
			api_set_kernel_sp(next->attr.kstack_tail);
		}

		if (next != fpu_holder) {
			if (fpu_holder)
				fpu_save(&fpu_holder);

			fpu_restore(&next);
			fpu_holder = next;
		}
	}

	stack_switch_wrapper(&(prev->mpu.esp0), &(next->mpu.esp0));
}

void create_context(thread_t *th)
{
	VP_INT *sp = th->attr.kstack_tail;

	if (is_kthread(th)) {
		*--sp = th->attr.arg;
		*--sp = (INT) thread_end;
		th->mpu.esp0 = context_create_kernel(
				sp,
				EFLAGS_INTERRUPT_ENABLE | EFLAGS_IOPL_3,
				th->attr.entry);
	} else {
		th->mpu.esp0 = context_create_user(
				th->attr.kstack_tail,
				EFLAGS_INTERRUPT_ENABLE | EFLAGS_IOPL_3,
				th->attr.entry,
				th->attr.ustack_top);
		//TODO initialize and save fpu if user thread
	}
}
