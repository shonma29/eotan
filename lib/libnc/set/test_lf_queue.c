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
#if 0
#include <stdio.h>
#include <stddef.h>
#include <set/lf_stack.h>
#include <set/lf_queue.h>

typedef struct {
	lfq_pointer_t next;
	unsigned int value;
} node_t;

int main(int argc, char **argv)
{
	char buf[lfs_buf_size(lfq_node_size(4), 4)];
	lfq_node_t *x;
	lfq_t q;
	lfs_t s;
	int r;
	int y;

	lfs_initialize(&s, buf, lfq_node_size(4), 4);
	lfq_initialize(&q, &s, 4);

	// init
	printf("init[0]=%p, %p, %p\n", &q, &s, &buf[0]);

	// empty
	y = 0;
	r = lfq_dequeue(&y, &q);
	printf("deq[0]=%x, %x\n", r, y);

	// enq 1
	y = 1;
	r = lfq_enqueue(&q, &y);
	printf("enq[1]=%x, %x\n", r, y);

	// deq 1
	y = 2;
	r = lfq_dequeue(&y, &q);
	printf("deq[1]=%x, %x\n", r, y);

	y = 3;
	r = lfq_dequeue(&y, &q);
	printf("deq[1]=%x, %x\n", r, y);

	// enq 2
	y = 4;
	r = lfq_enqueue(&q, &y);
	printf("enq[2]=%x, %x\n", r, y);

	y = 5;
	r = lfq_enqueue(&q, &y);
	printf("enq[2]=%x, %x\n", r, y);

	// deq 2
	y = 6;
	r = lfq_dequeue(&y, &q);
	printf("deq[2]=%x, %x\n", r, y);

	y = 7;
	r = lfq_dequeue(&y, &q);
	printf("deq[2]=%x, %x\n", r, y);

	y = 8;
	r = lfq_dequeue(&y, &q);
	printf("deq[2]=%x, %x\n", r, y);

	return 0;
}
#endif
