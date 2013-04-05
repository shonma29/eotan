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
#include <stddef.h>
#include <boot/modules.h>
#include <mpu/memory.h>
#include "func.h"
#include "thread.h"
#include "mpu/mpufunc.h"

static inline PTE *getPageDirectory(const T_TCB *th)
{
	return (PTE*)(th->mpu.context.cr3);
}

extern int isValidModule(const Elf32_Ehdr *eHdr);

static ER run(const UW type, const Elf32_Ehdr *eHdr);
static ER dupModule(T_TCB *th, const Elf32_Ehdr *eHdr);
static ER alloc(T_TCB *th, UB **allocated, UB *start, const size_t len);
static void set_initrd(ModuleHeader *h);
static void release(const void *head, const void *end);


void run_init_program(void)
{
	ModuleHeader *h = (ModuleHeader*)(MODULES_ADDR | MIN_KERNEL);

	while (h->type != mod_end) {
		Elf32_Ehdr *eHdr;
		UW addr;

		switch (h->type) {
		case mod_server:
		case mod_user:
			eHdr = (Elf32_Ehdr*)&(h[1]);
			run(h->type, eHdr);
			break;

		case mod_initrd:
			set_initrd(h);
			break;

		default:
			break;
		}

		addr = (UW)h + sizeof(*h) + h->length;
		h = (ModuleHeader*)addr;
	}

	release((void*)MODULES_ADDR,
			kern_v2p((void*)((UW)h + sizeof(*h))));
}

static ER run(const UW type, const Elf32_Ehdr *eHdr)
{
	ER err;
	ID tskId;
	T_CTSK pk_ctsk = {
		NULL,
		TA_HLNG,
		(FP)(eHdr->e_entry),
		KERNEL_LEVEL,
		KERNEL_STACK_SIZE,
		NULL
	};

	err = new_task(&pk_ctsk, &tskId, FALSE);
	if (err) {
		printk("[KERN] new_task error(%d)\n", err);
		return err;
	}

	if (type == mod_user) {
		T_TCB *th = get_thread_ptr(tskId);

		if (!th) {
			//TODO really happen?
			printk("[KERN] task(%d) not found\n", tskId);
			return E_SYS;
		}

		err = dupModule(th, eHdr);

		//TODO rollback on error;
		if (!err)
			set_autorun_context(th);
	}

	if (!err) {
		thread_start(tskId, 0);
		thread_switch();
	}

	return err;
}

static ER dupModule(T_TCB *th, const Elf32_Ehdr *eHdr)
{
	Elf32_Phdr *pHdr;
	size_t i;
	UB *allocated = NULL;

	if (!isValidModule(eHdr)) {
		printk("[KERN] bad module\n");
		return E_SYS;
	}

	pHdr = (Elf32_Phdr*)&(((UB*)eHdr)[eHdr->e_phoff]);

	for (i = 0; i < eHdr->e_phnum; pHdr++, i++) {
		UB *w;
		ER err;
#ifdef DEBUG
		printk("t=%d o=%p v=%p p=%p"
				", f=%d, m=%d\n",
				pHdr->p_type, pHdr->p_offset,
				pHdr->p_vaddr, pHdr->p_paddr,
				pHdr->p_filesz, pHdr->p_memsz);
#endif
		if (pHdr->p_type != PT_LOAD)
			continue;

		w = (UB*)(pHdr->p_vaddr);

		err = alloc(th, &allocated, w, pHdr->p_memsz);
		if (err)
			return err;

		/* pages gotten by palloc are zero cleared */
		vmemcpy(th, w, (UB*)eHdr + pHdr->p_offset,
				pHdr->p_filesz);
	}

	return E_OK;
}

static ER alloc(T_TCB *th, UB **allocated, UB *start, const size_t len)
{
	UB *last = roundDown(*allocated);
	UB *end = roundUp(start + len);

	start = last? last:roundDown(start);

	for (; (UW)start < (UW)end; start += PAGE_SIZE) {
		void *p = palloc(1);

		if (!p) {
			printk("[KERN] no memory for user\n");
			return E_NOMEM;
		}

		if (!vmap(th, (UW)start, (UW)p, ACC_USER)) {
			printk("[KERN] vmap error(%p, %p)\n", start, p);
			return E_SYS;
		}
	}

	*allocated = end;

	return E_OK;
}

static void set_initrd(ModuleHeader *h)
{
	machineInfo.rootfs = 0x80020000;
	machineInfo.initrd_start = (UW)&(h[1]);
	machineInfo.initrd_size = h->bytes;
}

static void release(const void *head, const void *end)
{
	UW addr = (UW)head & ~((1 << PAGE_SHIFT) - 1);
	UW max = pages((UW)end - addr);
	size_t i;

	printk("[KERN] release addr=%p max=%d\n", addr, max);

	for (i = 0; i < max; i++) {
		pfree((void*)addr, 1);
		addr += PAGE_SIZE;
	}
}
