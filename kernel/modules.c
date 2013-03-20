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
#include <string.h>
#include <boot/modules.h>
#include <mpu/config.h>
#include "func.h"
#include "mpu/mpufunc.h"

#define USER_ADDR 0x00400000

extern int isValidModule(Elf32_Ehdr *eHdr);

static size_t dup(UB **to, Elf32_Ehdr *eHdr);
static ER run(const UW type, const Elf32_Ehdr *eHdr, UB *to,
		const size_t size);
static ER map(const ID tskId, UB *to, UB *from, const size_t size);


void run_init_program(void)
{
	ModuleHeader *h = (ModuleHeader*)MODULES_ADDR;

	while (h->type != mod_end) {
		Elf32_Ehdr *eHdr;
		UW addr;
		UB *to = NULL;
		size_t size;

		switch (h->type) {
		case mod_server:
		case mod_user:
			eHdr = (Elf32_Ehdr*)&(h[1]);
			size = dup(&to, eHdr);
			if (size)
				run(h->type, eHdr, to, size);
			break;

		default:
			break;
		}

		addr = (UW)h + sizeof(*h) + h->length;
		h = (ModuleHeader*)addr;
	}
}

static size_t dup(UB **to, Elf32_Ehdr *eHdr)
{
	Elf32_Phdr *pHdr;
	UB *p;
	size_t i;
	size_t size = 0;

	if (!isValidModule(eHdr)) {
		printk("bad module\n");
		return 0;
	}

	p = (UB*)eHdr;
	pHdr = (Elf32_Phdr*)&(p[eHdr->e_phoff]);

	for (i = 0; i < eHdr->e_phnum; pHdr++, i++) {
		UB *w;
		UB *r;
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
		if (i == 0)
			*to = w;

		if ((UW)w < MIN_KERNEL)
			w += USER_ADDR;

		r = (UB*)eHdr + pHdr->p_offset;
		memcpy(w, r, pHdr->p_filesz);
		memset(w + pHdr->p_filesz, 0,
				pHdr->p_memsz - pHdr->p_filesz);
		size = pHdr->p_vaddr - (UW)(*to) + pHdr->p_memsz;
	}

	return size;
}


static ER run(const UW type, const Elf32_Ehdr *eHdr, UB *to,
		const size_t size)
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
		err = map(tskId, to, to + USER_ADDR,
				(size + PAGE_SIZE - 1) / PAGE_SIZE);
	}

	if (!err) {
		thread_start(tskId, 0);
		thread_switch();
	}

	return err;
}

static ER map(const ID tskId, UB *to, UB *from, const size_t size)
{
	T_TCB *p = get_thread_ptr(tskId);
	UW i;

	if (!p) {
		printk("[KERN] task(%d) not found\n", tskId);
		return E_SYS;
	}

	for (i = 0; i < size; i++) {
		if (!vmap(p, (UW)to, (UW)from, ACC_USER)) {
			printk("[KERN] vmap error(%p, %p)\n", to, from);
			return E_SYS;
		}

		from += PAGE_SIZE;
		to += PAGE_SIZE;
	}

	set_autorun_context(p);

	return E_OK;
}

