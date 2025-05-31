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
#include <stddef.h>
#include <stdnoreturn.h>
#include <mpufunc.h>
#include <mpu/io.h>
#include <mpu/memory.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include <starter/modules.h>

static void callback(void);
static ER run_module(const int, const void *, const PRI);
static void release_pages(const void *, const void *);


noreturn void run_kernel(void)
{
	for (ModuleHeader *h = (ModuleHeader *) MODULES_ADDR;
			h->type != mod_end;) {
		if (h->type == mod_kernel) {
			void (*entry)(void *) = (void *) (h->entry);
			entry((void *) callback);
			break;
		}

		h = (ModuleHeader *) ((uintptr_t) h + sizeof(*h) + h->length);
	}

	for (;;);
}

static void callback(void)
{
	ModuleHeader *h = (ModuleHeader *) (kern_p2v((void *) MODULES_ADDR));
	while (h->type != mod_end) {
		switch (h->type) {
		case mod_kthread:
			run_module(h->arg, h->entry, pri_server_high);
			break;
		case mod_server:
			run_module(h->arg, h->entry, pri_server_middle);
			break;
		default:
//TODO set device names
//TODO set env
//TODO authenticate
//TODO run 'init' at last, and init exec shell
			break;
		}

		h = (ModuleHeader *) ((uintptr_t) h + sizeof(*h) + h->length);
	}

	di();
	paging_reset();
	release_pages((void *) (CORE_STACK_ADDR - CORE_STACK_SIZE),
			(void *) CORE_STACK_ADDR);
	release_pages((void *) BOOT_ADDR, (void *) ((uintptr_t) h + sizeof(*h)));
}

static ER run_module(const int tid, const void *entry, const PRI priority)
{
	T_CTSK pk_ctsk = {
		TA_HLNG | TA_ACT,
		(VP_INT) NULL,
		(FP) entry,
		priority,
		KTHREAD_STACK_SIZE,
		NULL,
		NULL,
		NULL
	};
//TODO give information of boot device to 'fs'
	ER result = kcall->thread_create(tid, &pk_ctsk);
	if (result)
		kcall->printk("thread_create error(%d)\n", result);

	return result;
}

static void release_pages(const void *head, const void *end)
{
	uintptr_t addr = (uintptr_t) head & ~((1 << BITS_OFFSET) - 1);
	size_t max = pages((uintptr_t) end - addr);
#ifdef DEBUG
	kcall->printk("release addr=%p pages=%d\n", (void *) addr, max);
#endif
	for (int i = 0; i < max; i++) {
		kcall->pfree((void *) addr);
		addr += PAGE_SIZE;
	}
}
