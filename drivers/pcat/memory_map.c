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
#include <limits.h>
#include <string.h>
#include <boot/memory.h>
#ifdef USE_VESA
#include <vesa.h>
#endif
#include <boot/modules.h>
#include <mpu/desc.h>
#include <mpu/memory.h>
#include <mpu/mpufunc.h>
#include <nerve/config.h>
#include <nerve/memory_map.h>

#define DPAGE_OFFSET_MASK (4096ULL - 1ULL)
#define DPAGE_ADDR_MASK ~(4096ULL - 1ULL);
#define OVER_INT 0x100000000ULL

static MemoryMap *mm = (MemoryMap*)MEMORY_MAP_ADDR;

extern int printk(const char *format, ...);

static ER map_initialize(const size_t pages);
static ER map_set_use(const void *addr, const size_t pages);
#ifdef DEBUG
static void map_print(void);
#endif
static UW getLastPresentAddress(void);
static void setAbsentPages(void);
static UW getMemoryRange(MemoryInfo *p);
static void *setModules(void);
static size_t dupModule(UB **to, Elf32_Ehdr *eHdr);


void memory_initialize(void)
{
	/* create memory map */
	map_initialize(pages(getLastPresentAddress()));
	setAbsentPages();

	/* keep realmode interrupt vector, BIOS workarea */
	map_set_use(0, 1);
	/* keep GDT, IDT */
	map_set_use(kern_v2p((void*)GDT_ADDR), 1);
	/* keep page directory */
	map_set_use((void*)KTHREAD_DIR_ADDR, 1);
	/* keep boot infomation */
	map_set_use(kern_v2p((void*)BOOT_INFO_ADDR), 1);
	/* keep kernel stack */
	map_set_use(kern_v2p((void*)(CORE_STACK_ADDR - CORE_STACK_SIZE)),
			pages(CORE_STACK_SIZE));
	/* keep modules */
	map_set_use((void*)MODULES_ADDR,
			pages((UW)setModules() - MODULES_ADDR));

	/* keep kernel log */
	map_set_use(kern_v2p((void*)KERNEL_LOG_ADDR), pages(KERNEL_LOG_SIZE));
	/* keep memory map */
	map_set_use(kern_v2p((void*)mm),
			pages(mm->max_blocks * sizeof(UW) + sizeof(*mm)));
#ifdef DEBUG
	map_print();
#endif
}

static ER map_initialize(const size_t pages)
{
	UW i;
	UW bits;

	mm->left_pages = (pages > MAX_PAGES)? MAX_PAGES:pages;
	mm->last_block = 0;
	mm->max_blocks = (mm->left_pages + INT_BIT - 1) >> MPU_LOG_INT;
	mm->max_pages = mm->left_pages;

	for (i = 0; i < mm->max_blocks; i++)
		mm->map[i] = MAP_FULL_FREE;

	bits = mm->left_pages & BITS_MASK;
	if (bits)
		mm->map[i - 1] = (1 << bits) - 1;

	printk("map_initialize left=%d last=%d max=%d\n",
			mm->left_pages, mm->last_block, mm->max_blocks);

	return E_OK;
}

static ER map_set_use(const void *addr, const size_t pages)
{
	UW i;
	UW bits;
	UW left;
	UW mask;

#ifdef DEBUG
	printk("map_set_use addr=%p pages=%d\n", addr, pages);
#endif
	if (!pages)
		return E_PAR;

	i = (UW)addr >> BITS_OFFSET;
	if (i >= mm->max_pages)
		return E_PAR;

	left = (i + pages > mm->max_pages)? (mm->max_pages - i):pages;
	mm->left_pages -= left;
	bits = i & BITS_MASK;
	i >>= MPU_LOG_INT;

	if (bits) {
		mask = (1 << bits) - 1;

		if (bits + left < INT_BIT) {
			left += bits;
		} else {
			mm->map[i++] &= mask;
			left -= INT_BIT - bits;
			mask = 0;
		}
	} else {
		mask = 0;
	}

	bits = left & BITS_MASK;
	left = (left >> MPU_LOG_INT) + i;

	for (; i < left; i++)
		mm->map[i] = MAP_FULL_USE;

	if (bits)
		mm->map[i] &= mask | ~((1 << bits) - 1);

	return E_OK;
}

#ifdef DEBUG
static void map_print(void)
{
	size_t i;

	for (i = 0; i < mm->max_blocks; i++) {
		if (i && !(i & (8 - 1)))
			printk("\n");

		printk("%x ", mm->map[i]);
	}

	printk("\n");
}
#endif

static UW getLastPresentAddress(void)
{
	UW max = *((UW*)MEMORY_INFO_END);
	MemoryInfo *p = (MemoryInfo*)MEMORY_INFO_ADDR;
	UW prevEnd = 0;
#ifdef USE_VESA
	VesaInfo *v = (VesaInfo*)VESA_INFO_ADDR;
#endif
	for (; (UW)p < max; p ++) {
		size_t len;
		UW addr;

		printk("memory type=%x base=%x %x size=%x %x\n",
				p->type, p->baseHigh, p->baseLow,
				p->sizeHigh, p->sizeLow);

		if (p->type != MEMORY_PRESENT)
			continue;

		addr = p->baseLow;
#ifdef USE_VESA
		if (addr == v->buffer_addr)
			continue;
#endif
		len = getMemoryRange(p);
		if (!len)
			continue;

		prevEnd = addr + len * PAGE_SIZE;
	}

	return prevEnd;
}

static void setAbsentPages(void)
{
	UW max = *((UW*)MEMORY_INFO_END);
	MemoryInfo *p;
	UW prevEnd = 0;

	for (p = (MemoryInfo*)MEMORY_INFO_ADDR; (UW)p < max; p++) {
		UW addr;
		size_t len = getMemoryRange(p);

		if (!len)
			continue;

		addr = p->baseLow;

		if (addr != prevEnd)
			map_set_use((void*)prevEnd, pages(addr - prevEnd));

		prevEnd = addr + len * PAGE_SIZE;

		if (p->type != MEMORY_PRESENT)
			map_set_use((void*)addr, len);
	}
}

static UW getMemoryRange(MemoryInfo *p)
{
	UD base;
	UD size;
	UD end;
	UD diff;

	if (p->baseHigh)
		return 0;

	base = ((UD)(p->baseHigh) << 32) | (UD)(p->baseLow);
	size = ((UD)(p->sizeHigh) << 32) | (UD)(p->sizeLow);
	end = ((UD)base + (UD)size) & DPAGE_ADDR_MASK;

	if ((UD)base & DPAGE_OFFSET_MASK)
		base = (UD)base & DPAGE_ADDR_MASK;

	if (base > end)
		return 0;

	diff = (UD)end - (UD)base;
	if (end > OVER_INT)
		diff -= (UD)(end - OVER_INT);

	p->baseLow = (UD)base & 0xffffffff;

	return (((UD)diff + PAGE_SIZE - 1) >> BITS_OFFSET) & 0xffffffff;
}

static void *setModules(void)
{
	ModuleHeader *h = (ModuleHeader*)MODULES_ADDR;
	size_t i;

	for (i = 0; h->type != mod_end; i++) {
		Elf32_Ehdr *eHdr;
		UW addr;
		UB *to = NULL;
		size_t size;

		printk("module type=%x length=%x bytes=%x zBytes=%x\n",
				h->type, h->length, h->bytes, h->zBytes);

		switch (h->type) {
		case mod_kernel:
		case mod_driver:
		case mod_server:
			eHdr = (Elf32_Ehdr*)&(h[1]);
			size = dupModule(&to, eHdr);
			if (size)
				map_set_use(kern_v2p((void*)to), pages(size));
			break;

		case mod_user:
		case mod_initrd:
		default:
			break;
		}

		addr = (UW)h + sizeof(*h) + h->length;
		h = (ModuleHeader*)addr;
	}

	return (void*)((UW)h + sizeof(*h));
}

static size_t dupModule(UB **to, Elf32_Ehdr *eHdr)
{
	Elf32_Phdr *pHdr;
	UB *p;
	size_t i;
	size_t size = 0;

	if (!isValidModule(eHdr))
		panic("bad ELF");

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

		r = (UB*)eHdr + pHdr->p_offset;
		//TODO fill a gap with zero
		memcpy(w, r, pHdr->p_filesz);
		memset(w + pHdr->p_filesz, 0, pHdr->p_memsz - pHdr->p_filesz);
		size = pHdr->p_vaddr - (UW)(*to) + pHdr->p_memsz;
	}

	return size;
}
