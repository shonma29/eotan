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

int main(int argc, char **argv)
{
	char buf[stack_buf_size(16, 4)];
	lf_stack_t s;
	void *p;
	void *x[4];
int i;
	// init
	printf("init buf=%p, size=%d\n", buf, sizeof(buf) / sizeof(char));
	stack_initialize(&s, buf, 16, 4);
printf("%p\n", s.head.next);
for (i = 0; i < 4; i++) {
	char *q = &buf[i * 20];
	int *r = (int*)q;
	printf("%x\n", *r);
}
	// empty
	p = stack_pop(&s);
	printf("pop[0]=%p\n", p);
	x[0] = p;
	p = stack_pop(&s);
	printf("pop[0]=%p\n", p);
	x[1] = p;
	p = stack_pop(&s);
	printf("pop[0]=%p\n", p);
	x[2] = p;
	p = stack_pop(&s);
	printf("pop[0]=%p\n", p);
	x[3] = p;
	p = stack_pop(&s);
	printf("pop[0]=%p\n", p);

	// inner
	printf("next[0]=%p\n", stack_next(x[0]));
	printf("buf[0]=%p\n", stack_buf(x[0]));

	// push
	buf[0] = 1;
	buf[1] = 2;
	buf[2] = 3;
	buf[3] = 4;
	buf[4] = 5;
	stack_push(&s, x[0]);
	p = stack_pop(&s);
	printf("pop[1]=%p\n", p);
	p = stack_pop(&s);
	printf("pop[1]=%p\n", p);

	printf("next[1]=%p\n", stack_next(x[0]));
	printf("buf[1]=%p\n", stack_buf(x[0]));
	printf("%x, %x, %x, %x, %x\n",
		buf[0], buf[1], buf[2], buf[3], buf[4]);

	// push 2
	stack_push(&s, x[0]);
	stack_push(&s, x[1]);
	stack_push(&s, x[2]);
	stack_push(&s, x[3]);
	p = stack_pop(&s);
	printf("pop[2]=%p\n", p);
	p = stack_pop(&s);
	printf("pop[2]=%p\n", p);
	p = stack_pop(&s);
	printf("pop[2]=%p\n", p);
	p = stack_pop(&s);
	printf("pop[2]=%p\n", p);
	p = stack_pop(&s);
	printf("pop[2]=%p\n", p);

	return 0;
}
#endif
