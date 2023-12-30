#ifndef SET_LF_QUEUE_H
#define SET_LF_QUEUE_H
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
#include <set/lf_stack.h>

#define QUEUE_OK (0)
#define QUEUE_EMPTY (-1)
#define QUEUE_MEMORY (-2)

typedef struct _lfq_pointer_t {
	struct _lfq_node_t *ptr;
	unsigned long count;
} lfq_pointer_t;

typedef struct _lfq_node_t {
	lfq_pointer_t next;
	char value[0];
} lfq_node_t;

typedef struct _lfq_t {
	lfq_pointer_t head;
	lfq_pointer_t tail;
	lfs_t stack;
	size_t size;
} lfq_t;


#define lfq_node_size(size) \
	(size + sizeof(lfq_node_t))

#define lfq_buf_size(size, entry_num) \
	(lfq_node_size(size) * entry_num)


extern int lfq_initialize(volatile lfq_t *q, void *buf,
		size_t size, size_t node_num);

extern int lfq_enqueue(volatile lfq_t *q, void *value);
extern int lfq_dequeue(volatile lfq_t *q, void *value);

#endif
