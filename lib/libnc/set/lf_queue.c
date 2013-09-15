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
#include <string.h>
#include <mpu/mutex.h>
#include <set/lf_queue.h>
#include <set/lf_stack.h>


int lfq_initialize(lfq_t *q, lfs_t *stack, size_t size)
{
	lfq_node_t *node = (lfq_node_t*)lfs_pop(stack);

	if (!node)
		return QUEUE_MEMORY;

	node->next.ptr = NULL;
	q->tail.ptr = q->head.ptr = node;
	q->stack = stack;
	q->size = size;

	return QUEUE_OK;
}

int lfq_enqueue(volatile lfq_t *q, void *value)
{
	lfq_node_t *node = (lfq_node_t*)lfs_pop(q->stack);

	if (!node)
		return QUEUE_MEMORY;

	node->next.ptr = NULL;
	memcpy(node->value, (char*)value, q->size);

	for (;;) {
		lfq_pointer_t tail = q->tail;
		lfq_pointer_t next = tail.ptr->next;

		if ((tail.ptr == q->tail.ptr)
				&& (tail.count == q->tail.count)) {
			if (next.ptr)
				cas64((char*)&(q->tail),
					tail.count,
					(unsigned int)tail.ptr,
					tail.count + 1,
					(unsigned int)next.ptr);
			else if (cas64((char*)&(tail.ptr->next),
					next.count,
					(unsigned int)next.ptr,
					next.count + 1,
					(unsigned int)node)) {
				cas64((char*)&(q->tail),
						tail.count,
						(unsigned int)tail.ptr,
						tail.count + 1,
						(unsigned int)node);
				return QUEUE_OK;
			}
		}
	}
}

int lfq_dequeue(void *value, volatile lfq_t *q)
{
	for (;;) {
		lfq_pointer_t head = q->head;
		lfq_pointer_t tail = q->tail;
		lfq_pointer_t next = head.ptr->next;

		if ((head.ptr == q->head.ptr)
				&& (head.count == q->head.count)) {
			if (head.ptr == tail.ptr) {
				if (!next.ptr)
					return QUEUE_EMPTY;

				cas64((char*)&(q->tail),
						tail.count,
						(unsigned int)tail.ptr,
						tail.count + 1,
						(unsigned int)next.ptr);
			} else {
				memcpy((char*)value, next.ptr->value, q->size);

				if (cas64((char*)&(q->head),
						head.count,
						(unsigned int)head.ptr,
						head.count + 1,
						(unsigned int)next.ptr)) {
					lfs_push(q->stack, head.ptr);
					return QUEUE_OK;
				}
			}
		}
	}
}
