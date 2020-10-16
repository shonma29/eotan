#ifndef _SYS_TIME_H_
#define _SYS_TIME_H_
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
#include <sys/types.h>

//TODO mpu specific
#define TIMESPEC_OFFSET_NSEC (0x7fffffff - (1000 * 1000 * 1000) + 1)

typedef int clockid_t;

struct timespec {
	time_t tv_sec;
	int32_t tv_nsec;
};


static inline void timespec_set(volatile struct timespec *t,
		const int64_t *sec, const int32_t *nsec)
{
	t->tv_sec = *sec;
	t->tv_nsec = *nsec + TIMESPEC_OFFSET_NSEC;
}

static inline void timespec_get_sec(int64_t *sec, const struct timespec *t)
{
	*sec = t->tv_sec;
}

static inline void timespec_get_nsec(int32_t *nsec, const struct timespec *t)
{
	*nsec = t->tv_nsec - TIMESPEC_OFFSET_NSEC;
}

static inline int timespec_equals(const struct timespec *t1,
		const struct timespec *t2)
{
	return ((t1->tv_sec == t2->tv_sec)
			&& (t1->tv_nsec == t2->tv_nsec));
}

extern void timespec_add(volatile struct timespec *dest, const struct timespec *operand);
extern int timespec_compare(const struct timespec *t1,
		const struct timespec *t2);

#endif
