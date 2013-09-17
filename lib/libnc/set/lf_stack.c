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
#include <stddef.h>
#include <mpu/mutex.h>
#include <set/lf_stack.h>


void lfs_initialize(volatile lfs_t *stack, void *buf,
		size_t size, size_t entry_num)
{
	size_t i;
	lfs_entry_t *p = (lfs_entry_t*)buf;

	stack->head.next = p;
	stack->entry_size = lfs_entry_size(size);

	for (i = 0; i < entry_num - 1; i++) {
		char *q = (char*)p;
		lfs_entry_t *next =
				(lfs_entry_t*)(q + stack->entry_size);

		p->next = next;
		p = next;
	}

	p->next = NULL;
}

void lfs_push(volatile lfs_t *stack, void *p)
{
	lfs_entry_t *entry = (lfs_entry_t*)p;
	lfs_head_t newhead;

	newhead.next = entry;

	for (;;) {
		lfs_head_t oldhead = stack->head;

		if ((oldhead.next == stack->head.next)
				&& (oldhead.count == stack->head.count)) {
			entry->next = oldhead.next;
			newhead.count = oldhead.count + 1;

			if (cas64((char*)&(stack->head),
					(unsigned int)oldhead.count,
					(unsigned int)oldhead.next,
					(unsigned int)newhead.count,
					(unsigned int)newhead.next)) {
				break;
			}
		}
	}
}

void *lfs_pop(volatile lfs_t *stack) {
	for (;;) {
		lfs_head_t oldhead = stack->head;
		lfs_head_t newhead;

		if ((oldhead.next == stack->head.next)
				&& (oldhead.count == stack->head.count)) {
			if (!oldhead.next)	return NULL;

			newhead.next = oldhead.next->next;
			newhead.count = oldhead.count + 1;

			if (cas64((char*)&(stack->head),
					(unsigned int)oldhead.count,
					(unsigned int)oldhead.next,
					(unsigned int)newhead.count,
					(unsigned int)newhead.next)) {
				return oldhead.next;
			}
		}
	}
}
