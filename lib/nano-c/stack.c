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
#include <set/stack.h>


size_t stack_entry_size(size_t size) {
	return (size + sizeof(stack_entry_t) + sizeof(ptr_t) - 1)
			& ~(sizeof(ptr_t) - 1);
}

size_t stack_buf_size(size_t entry_size, size_t entry_num) {
	return stack_entry_size(entry_size) * entry_num;
}

void stack_initialize(stack_t *guard, void *buf,
		size_t entry_size, size_t entry_num) {
	size_t i;
	stack_entry_t *p = (stack_entry_t*)buf;

	guard->head.next = p;
	guard->buf = buf;

	for (i = 0; i < entry_num - 1; i++) {
		char *q = (char*)p;
		stack_entry_t *next = (stack_entry_t*)(q + entry_size);

		p->next = next;
		p = next;
	}

	p->next = NULL;
}

void stack_push(stack_t *guard, void *p) {
	stack_entry_t *entry = (stack_entry_t*)p;
	stack_head_t newhead;

	newhead.next = entry;

	for (;;) {
		stack_head_t oldhead = guard->head;

		entry->next = oldhead.next;
		newhead.count = oldhead.count + 1;

		if (cas64((char*)&(guard->head),
				(unsigned int)oldhead.count,
				(unsigned int)oldhead.next,
				(unsigned int)newhead.count,
				(unsigned int)newhead.next)) {
			break;
		}
	}
}

void *stack_pop(stack_t *guard) {
	for (;;) {
		stack_head_t oldhead = guard->head;
		stack_head_t newhead;

		if (!oldhead.next)	return NULL;

		newhead.next = oldhead.next->next;
		newhead.count = oldhead.count + 1;

		if (cas64((char*)&(guard->head),
				(unsigned int)oldhead.count,
				(unsigned int)oldhead.next,
				(unsigned int)newhead.count,
				(unsigned int)newhead.next)) {
			return oldhead.next;
		}
	}
}
