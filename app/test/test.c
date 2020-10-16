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
#include <errno.h>
#include <local.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#if 0
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

static void test_fork_fpu(void)
{
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
}

static void teststrtol(void)
{
	char *x;
	char *y;

	x = "  -1/3";
	printf("%s %ld\n", x, strtol(x, NULL, 36));

	x = "  -1/3";
	printf("%s %ld %d\n", x, strtol(x, &y, 36),
			(uintptr_t) y - (uintptr_t) x);

	x = "  -1:3";
	printf("%s %ld %d\n", x, strtol(x, &y, 36),
			(uintptr_t) y - (uintptr_t) x);

	x = "  -1@3";
	printf("%s %ld %d\n", x, strtol(x, &y, 36),
			(uintptr_t) y - (uintptr_t) x);

	x = "  -1[3";
	printf("%s %ld %d\n", x, strtol(x, &y, 36),
			(uintptr_t) y - (uintptr_t) x);

	x = "  -0xa";
	printf("%s %ld %d\n", x, strtol(x, &y, 10),
			(uintptr_t) y - (uintptr_t) x);

	x = "  -1239A";
	printf("%s %ld %d\n", x, strtol(x, &y, 10),
			(uintptr_t) y - (uintptr_t) x);

	x = "-1af2";
	printf("%s %lx %d\n", x, strtol(x, &y, 16),
			(uintptr_t) y - (uintptr_t) x);

	x = "  +1239A";
	printf("%s %ld %d\n", x, strtol(x, &y, 10),
			(uintptr_t) y - (uintptr_t) x);

	x = "0AZ9";
	printf("%s %ld %d\n", x, strtol(x, &y, 36),
			(uintptr_t) y - (uintptr_t) x);

	x = "0az9";
	printf("%s %ld %d\n", x, strtol(x, &y, 36),
			(uintptr_t) y - (uintptr_t) x);

	x = "1234";
	printf("%s %ld %d\n", x, strtol(x, &y, 10),
			(uintptr_t) y - (uintptr_t) x);

	x = "0x1234";
	printf("%s %ld %d\n", x, strtol(x, &y, 10),
			(uintptr_t) y - (uintptr_t) x);

	x = "1AF2";
	printf("%s %lx %d\n", x, strtol(x, &y, 16),
			(uintptr_t) y - (uintptr_t) x);

	x = "0x1AF2";
	printf("%s %lx %d\n", x, strtol(x, &y, 16),
			(uintptr_t) y - (uintptr_t) x);

	x = "0X1AF2";
	printf("%s %lx %d\n", x, strtol(x, &y, 16),
			(uintptr_t) y - (uintptr_t) x);

	x = "1x1AF2";
	printf("%s %lx %d\n", x, strtol(x, &y, 16),
			(uintptr_t) y - (uintptr_t) x);

	x = "0y1AF2";
	printf("%s %lx %d\n", x, strtol(x, &y, 16),
			(uintptr_t) y - (uintptr_t) x);

	_set_local_errno(0);
	x = "01";
	printf("%s %ld %d %d\n", x, strtol(x, &y, 1),
			(uintptr_t) y - (uintptr_t) x, errno);

	x = "01";
	printf("%s %ld %d\n", x, strtol(x, &y, 2),
			(uintptr_t) y - (uintptr_t) x);

	x = "01";
	printf("%s %ld %d\n", x, strtol(x, &y, 36),
			(uintptr_t) y - (uintptr_t) x);

	_set_local_errno(0);
	x = "01";
	printf("%s %ld %d %d\n", x, strtol(x, &y, 37),
			(uintptr_t) y - (uintptr_t) x, errno);

	x = "0750";
	printf("%s %lo %d\n", x, strtol(x, &y, 8),
			(uintptr_t) y - (uintptr_t) x);

	x = "0x3122";
	printf("%s %lx %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x);

	x = "0X3122";
	printf("%s %lx %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x);

	x = "0y3122";
	printf("%s %lx %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x);

	x = "1x3122";
	printf("%s %lx %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x);

	x = "0750";
	printf("%s %lo %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x);

	x = "750";
	printf("%s %ld %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x);

	x = "0x0";
	printf("%s %lx %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x);

	_set_local_errno(0);
	x = "0x7FFFFFFF00";
	printf("%s %lx %d %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x, errno);

	x = "0x7FFFFFFF";
	printf("%s %lx %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x);

	_set_local_errno(0);
	x = "0x80000000";
	printf("%s %lx %d %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x, errno);

	x = "-0x80000000";
	printf("%s %lx %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x);

	_set_local_errno(0);
	x = "-0x80000001";
	printf("%s %lx %d %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x, errno);

	_set_local_errno(0);
	x = "0xffffffff";
	printf("%s %lx %d %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x, errno);

	_set_local_errno(0);
	x = "-0xffffffff";
	printf("%s %lx %d %d\n", x, strtol(x, &y, 0),
			(uintptr_t) y - (uintptr_t) x, errno);
}

static void test_local(void)
{
	// test thread local
	thread_local_t *local = _get_thread_local();
	printf("thread_local=%p, error_no=%d, thread_id=%d\n",
			local, local->error_no, local->thread_id);
}

static void test_printf(void)
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

	printf("%lld\n", (long long) lx);
}
#endif

int main(int argc, char **argv)
{
#if 0
	test_printf();
	test_local();
	test_fork_fpu();

	// test strtol
	teststrtol();

	// test exception
	int i = 1;
	int j = 0;
	printf("div %d\n", i / j);

	// test stack expansion (need no optimizer)
	printf("%d\n", recursion(1024));
#endif
	return 0;
}
