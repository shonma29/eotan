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
#ifdef USE_VESA
#include <vesa.h>
#endif
#include <arch/archfunc.h>
#include <boot/memory.h>
#include <mpu/desc.h>
#include <mpu/memory.h>
#include <nerve/config.h>

#define TYPE_SKIP (-1)

#define DPAGE_OFFSET_MASK (4096ULL - 1ULL)
#define DPAGE_ADDR_MASK ~(4096ULL - 1ULL)
#define OVER_INT 0x100000000ULL

extern int printk(const char *, ...);
extern int map_set_using(const void *, const size_t);

static size_t calc_length(const MemoryInfo *p);


size_t get_max_address(void)
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
		size_t len = calc_length(p);
		if (!len) {
			p->type = TYPE_SKIP;
			continue;
		}

		prevEnd = (head >> BITS_OFFSET) + len;
	}

	return prevEnd;
}

void set_reserved_pages(void)
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
			map_set_using((void*)(prevEnd << BITS_OFFSET),
					head - prevEnd);

		prevEnd = head + calc_length(p);
	}

	/* keep GDT, IDT */
	map_set_using(kern_v2p((void*)GDT_ADDR), 1);
	/* keep page directory */
	map_set_using((void*)KTHREAD_DIR_ADDR, 1);
}

static size_t calc_length(const MemoryInfo *p)
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
