#ifndef _MPU_ATOMIC_H_
#define _MPU_ATOMIC_H_
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
#include <stdint.h>

static inline int mutex_get(volatile char *mtx) {
	char v;

	__asm__ __volatile__ ( \
			"movb $1, %b0\n\t" \
			"xchgb %b0, (%1)\n\t" \
			:"=q"(v), "+m"(mtx));
	return v;
}

static inline void mutex_release(volatile char *mtx) {
	__asm__ __volatile__ ( \
			"xorb %%al, %%al\n\t" \
			"xchgb %%al, (%0)\n\t" \
			:"+m"(mtx) \
			: \
			:"%al");
}

static inline int cas2(char *p,
		uintptr_t old_high, uintptr_t old_low,
		uintptr_t new_high, uintptr_t new_low) {
	char eq;

	__asm__ __volatile__ ( \
			"lock cmpxchg8b %1\n\t" \
			"setz %0\n\t" \
			:"=q"(eq), "+m"(*p) \
			:"d"(old_high), "a"(old_low), \
					"c"(new_high), "b"(new_low) \
			:);
	return (int) eq;
}

#endif
