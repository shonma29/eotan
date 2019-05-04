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
#include <limits.h>
#include <major.h>
#include <stddef.h>
#include <string.h>
#include <arch/archfunc.h>
#include <boot/initrd.h>
#include <boot/modules.h>
#include <fs/fstype.h>
#include <mpu/memory.h>
#include <mpu/mpufunc.h>
#include <nerve/global.h>

static MemoryMap *mm = &(sysinfo->memory_map);

extern int printk(const char *, ...);
extern int decode(unsigned char *, const size_t);

static int map_initialize(const size_t pages);
int map_set_using(const void *addr, const size_t pages);
#ifdef DEBUG
static void map_print(void);
#endif

static void *set_modules(void);
static size_t dup_module(char **to, Elf32_Ehdr *eHdr);
static void initialize_initrd_info(void);
static void set_initrd_info(void);


void memory_initialize(void)
{
	/* create memory map */
	//TODO optimize
	map_initialize(get_max_address());
	set_reserved_pages();

	printk("memory_initialize pages=%d/%d clock=%d/%d\n",
			mm->rest_pages, mm->max_pages,
			mm->clock_block, mm->num_blocks);

	/* keep boot infomation */
	map_set_using(kern_v2p((void*)BOOT_INFO_ADDR), 1);
	/* keep memory map */
	map_set_using(kern_v2p((void*)mm),
			pages(mm->num_blocks * sizeof(mm->map[0])));
	/* keep kernel log */
	map_set_using(kern_v2p((void*)KERNEL_LOG_ADDR), pages(KERNEL_LOG_SIZE));
	/* keep initrd */
	map_set_using(kern_v2p((void*)INITRD_ADDR), pages(INITRD_SIZE));

	/* keep kernel stack */
	map_set_using(kern_v2p((void*)(CORE_STACK_ADDR - CORE_STACK_SIZE)),
			pages(CORE_STACK_SIZE));
	/* keep runner and modules */
	map_set_using(kern_v2p((void*)RUNNER_ADDR),
			pages((unsigned int)set_modules() - MODULES_ADDR) + 1);
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

int map_set_using(const void *addr, const size_t pages)
{
#ifdef DEBUG
	printk("map_set_using addr=%p pages=%d\n", addr, pages);
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

static void *set_modules(void)
{
	initialize_initrd_info();

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
			size_t size = dup_module(&to, eHdr);
			if (size)
				map_set_using(kern_v2p(to), pages(size));
		}
			break;
		case mod_initrd:
		{
			int result = decode((unsigned char*)&(h[1]), h->bytes);
			if (result)
				printk("failed to decode initrd %d\n", result);
			else
				set_initrd_info();
		}
			break;
		case mod_user:
		default:
			break;
		}

		unsigned int addr = (unsigned int)h + sizeof(*h) + h->length;
		h = (ModuleHeader*)addr;
	}

	return (void*)((unsigned int)h + sizeof(*h));
}

static size_t dup_module(char **to, Elf32_Ehdr *eHdr)
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

static void initialize_initrd_info(void)
{
	system_info_t *info = kern_v2p(sysinfo);
	info->root.device = get_device_id(DEVICE_MAJOR_ATA, 0);
	info->root.fstype = FS_SFS;
	info->initrd.start = NULL;
	info->initrd.size = 0;
}

static void set_initrd_info(void)
{
	system_info_t *info = kern_v2p(sysinfo);
	info->root.device = get_device_id(DEVICE_MAJOR_RAMDISK, 0);
	info->root.fstype = INITRD_FS;
	info->initrd.start = (void*)INITRD_ADDR;
	info->initrd.size = INITRD_SIZE;
}
