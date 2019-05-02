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
#include <nerve/global.h>
#include <nerve/memory_map.h>

#define TYPE_SKIP (-1)

#define DPAGE_OFFSET_MASK (4096ULL - 1ULL)
#define DPAGE_ADDR_MASK ~(4096ULL - 1ULL)
#define OVER_INT 0x100000000ULL

static MemoryMap *mm = &(sysinfo->memory_map);

extern int printk(const char *format, ...);

static int map_initialize(const size_t pages);
static int map_set_use(const void *addr, const size_t pages);
#ifdef DEBUG
static void map_print(void);
#endif
static size_t getLastPresentPage(void);
static void setAbsentPages(void);
static size_t getMemoryRange(MemoryInfo *p);
static void *setModules(void);
static size_t dupModule(char **to, Elf32_Ehdr *eHdr);


void memory_initialize(void)
{
	/* create memory map */
	//TODO optimize
	map_initialize(getLastPresentPage());
	setAbsentPages();

	printk("memory_initialize pages=%d/%d clock=%d/%d\n",
			mm->rest_pages, mm->max_pages,
			mm->clock_block, mm->num_blocks);

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
			pages((unsigned int)setModules() - MODULES_ADDR));

	/* keep kernel log */
	map_set_use(kern_v2p((void*)KERNEL_LOG_ADDR), pages(KERNEL_LOG_SIZE));
	/* keep memory map */
	map_set_use(kern_v2p((void*)mm),
			pages(mm->num_blocks * sizeof(mm->map[0])));
#ifdef DEBUG
	map_print();
#endif
}

static int map_initialize(const size_t pages)
{
	mm->rest_pages = (pages > MAX_PAGES)? MAX_PAGES:pages;
	mm->clock_block = 0;
	mm->num_blocks = (mm->rest_pages + INT_BIT - 1) >> MPU_LOG_INT;
	mm->max_pages = mm->rest_pages;
	mm->map = (unsigned int*)MEMORY_MAP_ADDR;

	unsigned int i;
	for (i = 0; i < mm->num_blocks; i++)
		mm->map[i] = MAP_ALL_FREE;

	unsigned int offset = mm->rest_pages & BITS_MASK;
	if (offset)
		mm->map[i - 1] = (1 << offset) - 1;

	return 0;
}

static int map_set_use(const void *addr, const size_t pages)
{
#ifdef DEBUG
	printk("map_set_use addr=%p pages=%d\n", addr, pages);
#endif
	if (!pages)
		return E_PAR;

	unsigned int i = (unsigned int)addr >> BITS_OFFSET;
	if (i >= mm->max_pages)
		return E_PAR;

	size_t left = (i + pages > mm->max_pages)? (mm->max_pages - i):pages;
	mm->rest_pages -= left;

	unsigned int mask;
	unsigned int offset = i & BITS_MASK;
	i >>= MPU_LOG_INT;

	if (offset) {
		mask = (1 << offset) - 1;

		if (offset + left < INT_BIT) {
			left += offset;
		} else {
			mm->map[i++] &= mask;
			left -= INT_BIT - offset;
			mask = 0;
		}
	} else
		mask = 0;

	offset = left & BITS_MASK;
	left = (left >> MPU_LOG_INT) + i;

	for (; i < left; i++)
		mm->map[i] = MAP_ALL_USING;

	if (offset)
		mm->map[i] &= mask | ~((1 << offset) - 1);

	return 0;
}

#ifdef DEBUG
static void map_print(void)
{
	for (unsigned int i = 0; i < mm->num_blocks; i++) {
		if (i && !(i & (8 - 1)))
			printk("\n");

		printk("%x ", mm->map[i]);
	}

	printk("\n");
}
#endif

static size_t getLastPresentPage(void)
{
	unsigned int prevEnd = 0;
#ifdef USE_VESA
	VesaInfo *v = (VesaInfo*)VESA_INFO_ADDR;
#endif
	size_t max = *((uint32_t*)MEMORY_INFO_END);
	for (MemoryInfo *p = (MemoryInfo*)MEMORY_INFO_ADDR;
			(size_t)p < max; p++) {
		printk("memory type=%x base=%x %x size=%x %x\n",
				p->type, p->baseHigh, p->baseLow,
				p->sizeHigh, p->sizeLow);

		if (p->type != MEMORY_PRESENT)
			continue;

		unsigned int head = p->baseLow;
#ifdef USE_VESA
		if (head == v->buffer_addr) {
			p->type = TYPE_SKIP;
			continue;
		}
#endif
		size_t len = getMemoryRange(p);
		if (!len) {
			p->type = TYPE_SKIP;
			continue;
		}

		prevEnd = (head >> BITS_OFFSET) + len;
	}

	return prevEnd;
}

static void setAbsentPages(void)
{
	unsigned int prevEnd = 0;
	size_t max = *((size_t*)MEMORY_INFO_END);
	for (MemoryInfo *p = (MemoryInfo*)MEMORY_INFO_ADDR;
			(size_t)p < max; p++) {
		if (p->type != MEMORY_PRESENT)
			continue;

		unsigned int head = p->baseLow >> BITS_OFFSET;
		if (p->baseLow & MASK_OFFSET)
			head++;

		if (head != prevEnd)
			map_set_use((void*)(prevEnd << BITS_OFFSET),
					head - prevEnd);

		prevEnd = head + getMemoryRange(p);
	}
}

static size_t getMemoryRange(MemoryInfo *p)
{
	if (p->baseHigh)
		return 0;

	uint_fast64_t base = (uint_fast64_t)(p->baseLow);
	uint_fast64_t rest = OVER_INT - base;
	uint_fast64_t size = ((uint_fast64_t)(p->sizeHigh) << 32)
			| (uint_fast64_t)(p->sizeLow);
	if (size > rest)
		size = rest;

	uint_fast64_t end = (base + size) & DPAGE_ADDR_MASK;

	if (p->baseLow & MASK_OFFSET) {
		base = (base + PAGE_SIZE) & DPAGE_ADDR_MASK;
		if (base >= OVER_INT)
			return 0;
	}
	if (base >= end)
		return 0;

	return (((end - base) >> BITS_OFFSET) & 0xffffffff);
}
//TODO move to starter
static void *setModules(void)
{
	ModuleHeader *h = (ModuleHeader*)MODULES_ADDR;
	for (int i = 0; h->type != mod_end; i++) {
		printk("module type=%x length=%x bytes=%x zBytes=%x\n",
				h->type, h->length, h->bytes, h->zBytes);

		switch (h->type) {
		case mod_kernel:
		case mod_driver:
		case mod_server:
		{
			Elf32_Ehdr *eHdr = (Elf32_Ehdr*)&(h[1]);
			char *to = NULL;
			size_t size = dupModule(&to, eHdr);
			if (size)
				map_set_use(kern_v2p(to), pages(size));
		}
			break;

		case mod_user:
		case mod_initrd:
		default:
			break;
		}

		unsigned int addr = (unsigned int)h + sizeof(*h) + h->length;
		h = (ModuleHeader*)addr;
	}

	return (void*)((unsigned int)h + sizeof(*h));
}

static size_t dupModule(char **to, Elf32_Ehdr *eHdr)
{
	size_t size = 0;

	if (!isValidModule(eHdr))
		panic("bad ELF");

	char *p = (char*)eHdr;
	Elf32_Phdr *pHdr = (Elf32_Phdr*)&(p[eHdr->e_phoff]);
	for (int i = 0; i < eHdr->e_phnum; pHdr++, i++) {
#ifdef DEBUG
		printk("t=%d o=%p v=%p p=%p"
				", f=%d, m=%d\n",
				pHdr->p_type, pHdr->p_offset,
				pHdr->p_vaddr, pHdr->p_paddr,
				pHdr->p_filesz, pHdr->p_memsz);
#endif
		if (pHdr->p_type != PT_LOAD)
			continue;

		char *w = (char*)(pHdr->p_vaddr);
		if (!i)
			*to = w;

		char *r = (char*)eHdr + pHdr->p_offset;
		//TODO fill a gap with zero
		memcpy(w, r, pHdr->p_filesz);
		memset(w + pHdr->p_filesz, 0, pHdr->p_memsz - pHdr->p_filesz);
		size = pHdr->p_vaddr - (unsigned int)(*to) + pHdr->p_memsz;
	}

	return size;
}
