#ifndef _CUNIT_H_
#define _CUNIT_H_
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
#include <stdio.h>
#include <stdlib.h>

#define __ITOA(n) #n
#define _ITOA(n) __ITOA(n)
#define _FAIL(msg) ("failed in " _ITOA(__LINE__) ". " msg)

#define assert_t(msg, exp) do { if (!(exp)) return _FAIL(msg); } while(0)
#define assert_f(msg, exp) do { if (exp) return _FAIL(msg); } while(0)
#define assert_eq(msg, a, b) do { if ((a) != (b)) return _FAIL(msg); } while(0)
#define assert_ne(msg, a, b) do { if ((a) == (b)) return _FAIL(msg); } while(0)

#define test(q) do { \
	char *msg = q(); \
	printf("%s %s.\n", #q, msg ? msg : "OK"); \
	if (msg) exit(1); \
} while(0)

#endif
