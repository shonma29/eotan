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
#include <elf.h>
#include <stddef.h>
#include <boot/modules.h>
#include <mpu/memory.h>
#include <nerve/config.h>
#include <nerve/kcall.h>

static ER run(const enum ModuleType type, const int tid,
		const Elf32_Ehdr *eHdr);
static void release_others(const void *head, const void *end);


void load_modules(void)
{
	ModuleHeader *h = (ModuleHeader*)(kern_p2v((void*)MODULES_ADDR));

	while (h->type != mod_end) {
		switch (h->type) {
		case mod_server:
			run(h->type, h->arg, (Elf32_Ehdr*)&(h[1]));
			break;
		default:
			break;
		}

		h = (ModuleHeader*)((unsigned int)h + sizeof(*h) + h->length);
	}

	release_others((void*)(CORE_STACK_ADDR - CORE_STACK_SIZE),
			(void*)CORE_STACK_ADDR);
	release_others(kern_p2v((void*)MODULES_ADDR),
			(void*)((unsigned int)h + sizeof(*h)));
}

static ER run(const enum ModuleType type, const int tid,
		const Elf32_Ehdr *eHdr)
{
	T_CTSK pk_ctsk = {
		TA_HLNG | TA_ACT,
		(VP_INT)NULL,
		(FP)(eHdr->e_entry),
		pri_server_middle,
		KTHREAD_STACK_SIZE,
		NULL,
		NULL,
		NULL
	};
	ER result = kcall->thread_create(tid, &pk_ctsk);
	if (result)
		kcall->printk("thread_create error(%d)\n", result);

	return result;
}

//TODO wait for init starting
static void release_others(const void *head, const void *end)
{
	unsigned int addr = (unsigned int)head & ~((1 << BITS_OFFSET) - 1);
	size_t max = pages((unsigned int)end - addr);

	kcall->printk("release addr=%p pages=%d\n", addr, max);

	for (int i = 0; i < max; i++) {
		kcall->pfree((void*)addr);
		addr += PAGE_SIZE;
	}
}
