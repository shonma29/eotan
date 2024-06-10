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
#include <major.h>
#include <string.h>
#include <arch/archfunc.h>
#include <fs/fstype.h>
#include <mpu/desc.h>
#include <mpu/memory.h>
#include <mpu/mpufunc.h>
#include <nerve/config.h>
#include <nerve/global.h>
#include <starter/initrd.h>
#include <starter/modules.h>
#include "starter.h"

#define INT_BIT ((CHAR_BIT) * sizeof(int))

static MemoryMap *mm = &(sysinfo->memory_map);

static int map_initialize(const size_t);
int map_set_using(const void *, const size_t);
#ifdef DEBUG
static void map_print(void);
#endif

static void *set_modules(void);
static void initialize_initrd_info(void);
static void set_initrd_info(const ModuleHeader *);


void memory_initialize(const size_t num_of_pages)
{
	// create memory map
	//TODO optimize
	map_initialize(num_of_pages);
	set_holes();

	printk("memory_initialize pages=%d/%d clock=%d/%d\n",
			mm->rest_pages, mm->max_pages,
			mm->clock_block, mm->num_blocks);

	// keep GDT, IDT
	map_set_using(kern_v2p((void *) GDT_ADDR), 1);
	// keep page directory
	map_set_using((void *) KTHREAD_DIR_ADDR, 1);

	// keep boot infomation
	map_set_using(kern_v2p((void *) BOOT_INFO_ADDR), 1);
	// keep memory map
	map_set_using(kern_v2p(mm->map),
			pages(mm->num_blocks * sizeof(mm->map[0])));
	// keep kernel log
	map_set_using(kern_v2p((void *) KERNEL_LOG_ADDR),
			pages(KERNEL_LOG_SIZE));

	// keep kernel stack
	map_set_using(kern_v2p((void *) (CORE_STACK_ADDR - CORE_STACK_SIZE)),
			pages(CORE_STACK_SIZE));
	// keep runner and modules
	map_set_using(kern_v2p((void *) BOOT_ADDR),
			pages((uintptr_t) set_modules()
					- (uintptr_t) kern_v2p(
							(void *) BOOT_ADDR)));
#ifdef DEBUG
	map_print();
#endif
}

static int map_initialize(const size_t pages)
{
	mm->rest_pages = pages;
	mm->clock_block = 0;
	mm->num_blocks = (mm->rest_pages + INT_BIT - 1) >> MPU_LOG_INT;
	mm->max_pages = mm->rest_pages;
	mm->map = (unsigned int *) MEMORY_MAP_ADDR;

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

	unsigned int i = (uintptr_t) addr >> BITS_OFFSET;
	if (i >= mm->max_pages)
		return E_PAR;

	size_t left = (i + pages > mm->max_pages) ? (mm->max_pages - i) : pages;
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

	ModuleHeader *h = (ModuleHeader *) MODULES_ADDR;
	while (h->type != mod_end) {
		printk("module type=%x length=%x bytes=%x\n",
				h->type, h->length, h->bytes);

		switch (h->type) {
		case mod_kernel:
		case mod_kthread:
		case mod_server:
		case mod_initrd:
		{
			int result = decode(h->address, &(h[1]), h->bytes,
					h->pages * PAGE_SIZE);
			if (result)
				printk("failed to decode module %d\n", result);
			else {
				if (h->type == mod_initrd)
					set_initrd_info(h);

				map_set_using(kern_v2p(h->address), h->pages);
			}
		}
			break;
		default:
			break;
		}

		h = (ModuleHeader *) ((uintptr_t) h + sizeof(*h) + h->length);
	}

	//TODO driver and initrd headers are not needed
	return (void *) ((uintptr_t) h + sizeof(*h));
}

static void initialize_initrd_info(void)
{
	system_info_t *info = kern_v2p(sysinfo);
	strcpy(info->root.device, DEVICE_CONTROLLER_ATA0);
	info->root.fstype = FS_TFS;
	info->root.block_size = 0;
	info->initrd.address = NULL;
	info->initrd.size = 0;
}

static void set_initrd_info(const ModuleHeader *header)
{
	system_info_t *info = kern_v2p(sysinfo);
	strcpy(info->root.device, DEVICE_CONTROLLER_RAMDISK0);
	info->root.fstype = INITRD_FS;
	info->root.block_size = INITRD_BLOCK_SIZE;
	info->initrd.address = header->address;
	info->initrd.size = header->pages * PAGE_SIZE;
}
