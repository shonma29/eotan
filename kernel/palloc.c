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
#include <mpu/bits.h>
#include <nerve/global.h>
#include <nerve/memory_map.h>
#include "func.h"

static void pzero(unsigned int *p);


void *palloc(void)
{
	MemoryMap *mm = &(sysinfo->memory_map);
	if (mm->rest_pages)
		for (int i = mm->clock_block; i < mm->num_blocks; i++) {
			unsigned int d = mm->map[i];
			if (d) {
				unsigned int bit = count_ntz(d);
				mm->map[i] &= ~(1 << bit);
				mm->clock_block = mm->map[i] ? i : (i + 1);
				mm->rest_pages--;

				unsigned int addr = ((i << MPU_LOG_INT) | bit)
						<< BITS_OFFSET;
				//TODO why here?
				addr = (unsigned int)kern_p2v((void*)addr);
				//TODO why here?
				pzero((unsigned int*)addr);

				return (void*)addr;
			}
		}

	return NULL;
}

static void pzero(unsigned int *p)
{
	for (int i = 0; i < PAGE_SIZE / sizeof(*p); i++)
		p[i] = 0;
}

void pfree(void *addr)
{
	addr = kern_v2p(addr);

	unsigned int i = (unsigned int)addr >> (BITS_OFFSET + MPU_LOG_INT);
	MemoryMap *mm = &(sysinfo->memory_map);
	if (i >= mm->num_blocks) {
		warn("pfree: over %p\n", addr);
		return;
	}

	unsigned int bit = 1 <<
			(((unsigned int)addr >> BITS_OFFSET) & BITS_MASK);
	if (mm->map[i] & bit) {
		warn("pfree: already free %p\n", addr);
		return;
	}

	mm->map[i] |= bit;

	if (i < mm->clock_block)
		mm->clock_block = i;

	mm->rest_pages++;
	return;
}
