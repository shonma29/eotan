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
#include <cga.h>
#include <elf.h>
#include <stdarg.h>
#include <string.h>
#include <boot/modules.h>
#include "../kernel/mpu/mpufunc.h"

#define INT_LEN (4)

#define BIOS_CURSOR_COL 0x0450
#define BIOS_CURSOR_ROW 0x0451
#define CGA_VRAM_ADDR 0x000b8000

extern int isValidModule(Elf32_Ehdr *eHdr);

static CGA_Console *cns;

static void console_initialize();
static W printk(const B *fmt, ...);
static ER kick(const ModuleHeader *h);


void _main(void)
{
	ModuleHeader *h = (ModuleHeader*)MODULES_ADDR;

	console_initialize();
	paging_initialize();
	gdt_initialize();
	idt_initialize();	

// get memory size
	while (h->type != mod_end) {
		UW addr;

		printk("%d %d %d %d\n",
				h->type, h->length, h->bytes, h->zBytes);

		if (h->type == mod_kernel) {
			kick(h);
			break;
		}

		addr = (unsigned int)h + sizeof(*h) + h->length;
		h = (ModuleHeader*)addr;
	}

	for(;;);
}

static void console_initialize()
{
	UB *x = (UB*)BIOS_CURSOR_COL;
	UB *y = (UB*)BIOS_CURSOR_ROW;

	cns = getConsole((const UH*)CGA_VRAM_ADDR);
	cns->locate(*x, *y);
	printk("Starter has woken up.\n");
}

static W printk(const B *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	return vnprintf((void*)(cns->putc), (char*)fmt, ap);
};

static ER kick(const ModuleHeader *h)
{
	Elf32_Ehdr *eHdr = (Elf32_Ehdr*)&(h[1]);
	Elf32_Phdr *pHdr;
	UB *p;
	size_t i;
	void (*entry)(void);

	if (!isValidModule(eHdr)) {
		printk("bad module\n");
		return E_PAR;
	}

	p = (UB*)eHdr;
	pHdr = (Elf32_Phdr*)&(p[eHdr->e_phoff]);

	for (i = 0; i < eHdr->e_phnum; pHdr++, i++) {
		UB *w;
		UB *r;

		printk("t=%d o=%p v=%p p=%p, fsz=%d, msz=%d\n",
				pHdr->p_type, pHdr->p_offset, pHdr->p_vaddr,
				pHdr->p_paddr, pHdr->p_filesz, pHdr->p_memsz);

		if (pHdr->p_type != PT_LOAD)
			continue;

		w = (unsigned char*)(pHdr->p_vaddr);
		r = (unsigned char*)eHdr + pHdr->p_offset;
		memcpy(w, r, pHdr->p_filesz);
		memset(w + pHdr->p_filesz, 0, pHdr->p_memsz - pHdr->p_filesz);
	}

	entry = (void*)(eHdr->e_entry);
	entry();

	return E_OK;
}
