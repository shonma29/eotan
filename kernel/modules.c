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
#include <elf.h>
#include <fstype.h>
#include <major.h>
#include <stddef.h>
#include <boot/modules.h>
#include <mpu/memory.h>
#include <nerve/config.h>
#include <nerve/global.h>
#include "func.h"
#include "ready.h"

static ER run(const UW type, const Elf32_Ehdr *eHdr);
static void set_initrd(ModuleHeader *h);
static void release_others(const void *head, const void *end);


void run_init_program(void)
{
	ModuleHeader *h = (ModuleHeader*)(MODULES_ADDR | MIN_KERNEL);

	while (h->type != mod_end) {
		switch (h->type) {
		case mod_server:
			run(h->type, (Elf32_Ehdr*)&(h[1]));
			break;

		case mod_initrd:
			set_initrd(h);
			break;

		default:
			break;
		}

		h = (ModuleHeader*)((UW)h + sizeof(*h) + h->length);
	}

	release_others((void*)MODULES_ADDR,
			kern_v2p((void*)((UW)h + sizeof(*h))));
}

static ER run(const UW type, const Elf32_Ehdr *eHdr)
{
	ID tskId;
	T_CTSK pk_ctsk = {
		TA_HLNG,
		NULL,
		(FP)(eHdr->e_entry),
		pri_server_middle,
		KERNEL_STACK_SIZE,
		NULL,
		NULL
	};

	tskId = thread_create_auto(&pk_ctsk);
	if (tskId < 0) {
		printk("thread_create_auto error(%d)\n", tskId);
		return tskId;
	}

	thread_start(tskId);
	dispatch();

	return E_OK;
}

static void set_initrd(ModuleHeader *h)
{
	system_info_t *sysinfo = (system_info_t*)SYSTEM_INFO_ADDR;

	sysinfo->root.device = get_device_id(DEVICE_MAJOR_RAMDISK, 0);
	sysinfo->root.fstype = FS_SFS;
	sysinfo->initrd.start = &(h[1]);
	sysinfo->initrd.size = h->bytes;
}

//TODO wait for init starting
//TODO release others (BIOS workarea, kernel stack, ...)
static void release_others(const void *head, const void *end)
{
	UW addr = (UW)head & ~((1 << PAGE_SHIFT) - 1);
	UW max = pages((UW)end - addr);
	size_t i;

	printk("release addr=%p max=%d\n", addr, max);

	for (i = 0; i < max; i++) {
		pfree((void*)addr);
		addr += PAGE_SIZE;
	}
}
