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
#include <limits.h>
#include <stddef.h>
#include <mpu/bits.h>
#include <mpu/memory.h>
#include <nerve/config.h>
#include <nerve/memory_map.h>
#include "func.h"
#include "sync.h"

static MemoryMap *mm = (MemoryMap*)MEMORY_MAP_ADDR;

static void pzero(UW *p);


void *palloc(void)
{
	size_t i;

	enter_serialize();
	if (mm->left_pages) {
		for (i = mm->last_block; i < mm->max_blocks; i++) {
			UW d = mm->map[i];

			if (d) {
				W bit = count_ntz(d);
				UW addr;

				mm->map[i] &= ~(1 << bit);
				mm->last_block = mm->map[i]? i:(i + 1);
				mm->left_pages--;
				leave_serialize();

				addr = ((i << MPU_LOG_INT) | bit)
						<< BITS_OFFSET;
				//TODO why here?
				addr = (UW)kern_p2v((void*)addr);
				//TODO why here?
				pzero((UW*)addr);

				return (void*)addr;
			}
		}
	}
	leave_serialize();

	return NULL;
}

static void pzero(UW *p)
{
	size_t i;

	for (i = 0; i < PAGE_SIZE / sizeof(UW); i++)
		p[i] = 0;
}

void pfree(void *addr)
{
	addr = kern_v2p(addr);
	size_t i = (size_t)addr >> (BITS_OFFSET + MPU_LOG_INT);
	UW bit;

	if (i >= mm->max_blocks) {
		printk("pfree: over %p\n", addr);
		return;// E_PAR;
	}

	bit = 1 << (((UW)addr >> BITS_OFFSET) & BITS_MASK);

	enter_serialize();
	if (mm->map[i] & bit) {
		leave_serialize();
		printk("pfree: already free %p\n", addr);
		return;// E_OBJ;
	}

	mm->map[i] |= bit;

	if (i < mm->last_block)
		mm->last_block = i;

	mm->left_pages++;
	leave_serialize();

	return;// E_OK;
}
