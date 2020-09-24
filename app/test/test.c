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
#include <local.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


static int recursion(int v)
{
	if ((v % 1000) == 0)
		printf("%d\n", v);

	if (v == 0)
		return 0;

	return (v + recursion(v - 1));
}

static void push(double x)
{
	__asm__ __volatile__ (\
		"fldl %0\n\t"\
		:"=m"(x)::);
}

static double pop(void)
{
	double y;
	__asm__ __volatile__ (\
		"fstl %0\n\t"\
		::"m"(y):);
	return y;
}

#if 0
static void teststrtol(void)
{
	char *x;
	char *y;

	x = "  -0xa";
	printf("%s %ld %p %p\n", x, strtol(x, &y, 10), x, y);

	x = "  -1234a";
	printf("%s %ld %p %p\n", x, strtol(x, &y, 10), x, y);

	x = "-1f42";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 16), x, y);

	x = "1F42";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 16), x, y);

	x = "0755";
	printf("%s %lo %p %p\n", x, strtol(x, &y, 8), x, y);

	x = "0755";
	printf("%s %lo %p %p\n", x, strtol(x, &y, 0), x, y);

	x = "0x3122";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 0), x, y);

	x = "0x0";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 0), x, y);

	x = "0x7FFFFFFF";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 0), x, y);

	x = "0x80000000";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 0), x, y);

	x = "0x80000001";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 0), x, y);

	x = "0xFFFFFFFF";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 0), x, y);

	x = "0x100000000";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 0), x, y);

	x = "-0x0";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 0), x, y);

	x = "-0x7FFFFFFF";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 0), x, y);

	x = "-0x80000000";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 0), x, y);

	x = "-0x80000001";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 0), x, y);

	x = "-0xFFFFFFFF";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 0), x, y);

	x = "-0x100000000";
	printf("%s %lx %p %p\n", x, strtol(x, &y, 0), x, y);
}
#endif

int main(int argc, char **argv)
{
	double x = 1.0;
	printf("%f\n", 0.0175);
	printf("%f\n", 0.175);
	printf("%f\n", 1.75);
	printf("%f\n", -17.5);
	printf("%f\n", sin(x));
	printf("%f\n", cos(x));

	long lx = 15;
	printf("%ld\n", lx);
	printf("%lo\n", lx);
	printf("%lx\n", lx);
#if 0
	printf("%lld\n", (long long) lx);

	// test exception
	int i = 1;
	int j = 0;
	printf("div %d\n", i / j);
#endif
	// test thread local
	thread_local_t *local = _get_thread_local();
	printf("thread_local=%p, error_no=%d, thread_id=%d\n",
			local, local->error_no, local->thread_id);

	// test stack expansion (need no optimizer)
	printf("%d\n", recursion(1024));

	// test fpu
	double v = 2.48;
	double w;
	w = pop();
	printf("%f %f\n", v, w);
	push(v);
	w = pop();
	printf("%f %f\n", v, w);
	sleep(1);
	w = pop();
	printf("%f %f\n", v, w);
		v = 7.13;
		push(v);
		w = pop();
		printf("parent: %f %f\n", v, w);

	pid_t pid = fork();
	if (pid == 0) {
		w = pop();
		printf("child: %f %f\n", v, w);
		v = 3.72;
		push(v);
		w = pop();
		printf("child: %f %f\n", v, w);
		exit(0);
	} else {
		int status;
		wait(&status);
		w = pop();
		printf("parent: %f %f\n", v, w);
		v = 5.86;
		push(v);
		w = pop();
		printf("parent: %f %f\n", v, w);
	}

#if 0
	// test strtol
	teststrtol();
#endif
	return 0;
}
