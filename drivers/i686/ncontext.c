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
#include <setting.h>
#include <func.h>
#include "gate.h"
#include "mpufunc.h"
#include "msr.h"
#include "tss.h"

VP *context_prev_sp;
VP *context_next_sp;
static W current_domain_id = KERNEL_DOMAIN_ID;

void context_initialize(void)
{
	tr_set(dummy_tss);
}

void context_set_kernel_sp(const VP addr)
{
	tss_t *tss = (tss_t*)TSS_ADDR;

	tss->esp0 = (UW)addr;
	msr_write(sysenter_esp_msr, (UW)addr);
}

VP_INT *context_create_kernel(VP_INT *sp, const UW eflags, const FP eip)
{
	VP_INT *esp0;

	/* eflags */
	*--sp = eflags;
	/* cs */
	*--sp = kern_code;
	/* eip */
	*--sp = (VP_INT)eip;
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
	*--sp = (VP_INT)esp0;
	/* ebp */
	*--sp = 0;
	/* esi */
	*--sp = 0;
	/* edi */
	*--sp = 0;

	return sp;
}

VP_INT *context_create_user(VP_INT *sp, const UW eflags, const FP eip,
		const VP esp)
{
	VP_INT *esp0;

	/* ss */
	*--sp = user_data | dpl_user;
	/* user esp */
	*--sp = (VP_INT)esp;
	/* eflags */
	*--sp = eflags;
	/* cs */
	*--sp = user_code | dpl_user;
	/* eip */
	*--sp = (VP_INT)eip;
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
	*--sp = (VP_INT)esp0;
	/* ebp */
	*--sp = 0;
	/* esi */
	*--sp = 0;
	/* edi */
	*--sp = 0;

	return sp;
}

void context_switch_page_table(thread_t *next)
{
	if ((next->attr.domain_id != KERNEL_DOMAIN_ID)
			&& (next->attr.domain_id != current_domain_id)) {
		paging_set_directory((VP)MPU_PAGE_TABLE(next));
		current_domain_id = next->attr.domain_id;
	}
}

void context_reset_page_table()
{
	paging_set_directory((VP)PAGE_DIR_ADDR);
	current_domain_id = KERNEL_DOMAIN_ID;
}

void context_reset_page_cache(const thread_t *task, const VP addr)
{
	if ((task->attr.domain_id == KERNEL_DOMAIN_ID)
			|| (task->attr.domain_id == current_domain_id)) {
		tlb_flush(addr);
	}
}

void create_context(thread_t *th)
{
	if (th->attr.domain_id == KERNEL_DOMAIN_ID) {
		VP_INT *sp = th->attr.kstack_tail;

		*--sp = th->attr.arg;
		*--sp = (INT)thread_end;
		th->mpu.context.esp0 = context_create_kernel(
				sp,
				EFLAG_IBIT | EFLAG_IOPL3,
				th->attr.entry);
		th->mpu.use_fpu = FALSE;

	} else
		th->mpu.use_fpu = TRUE;
}
