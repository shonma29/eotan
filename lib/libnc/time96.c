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
#include <sys/time.h>


void timespec_add(struct timespec *dest, const struct timespec *operand)
{
	dest->tv_sec += operand->tv_sec;
	dest->tv_nsec += operand->tv_nsec;

	if (dest->tv_nsec < 0) {
		dest->tv_sec++;
		dest->tv_nsec = (int)((dest->tv_nsec & 0x7fffffff)
				+ TIMESPEC_OFFSET_NSEC);
	}
}

int timespec_after(const struct timespec *t1, const struct timespec *t2)
{
	time_t diff = t1->tv_sec - t2->tv_sec;

	if (diff < 0)
		return 0;
	else if (diff > 0)
		return 1;
	else
		return t1->tv_nsec >= t2->tv_nsec;
}
