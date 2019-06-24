#ifndef _STDLIB_H_
#define _STDLIB_H_
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
#include <stdnoreturn.h>

#define RAND_MAX 0x7fffffff

#define EXIT_SUCCESS (0)
#define EXIT_FAILURE (1)

typedef struct {
	int quot;
	int rem;
} div_t;

typedef struct {
	long quot;
	long rem;
} ldiv_t;

typedef struct {
	long long quot;
	long long rem;
} lldiv_t;

static inline int abs(int value)
{
	return ((value < 0) ? (-value) : value);
}

static inline long labs(long value)
{
	return ((value < 0) ? (-value) : value);
}

static inline div_t div(int numerator, int denominator)
{
	div_t x = {
		x.quot = numerator / denominator,
		x.rem = numerator % denominator
	};

	return x;
}

static inline ldiv_t ldiv(long numerator, long denominator)
{
	ldiv_t x = {
		x.quot = numerator / denominator,
		x.rem = numerator % denominator
	};

	return x;
}

static inline lldiv_t lldiv(long long numerator, long long denominator)
{
	lldiv_t x = {
		x.quot = numerator / denominator,
		x.rem = numerator % denominator
	};

	return x;
}

extern int atoi(const char *);
extern long atol(const char *);
extern void *bsearch(const void *, const void *, size_t,
		size_t, int (*)(const void *, const void *));
extern void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
extern char *getenv(const char *);
extern int rand(void);
extern void srand(unsigned int);
extern void noreturn abort(void);
extern void noreturn exit(int);
extern void *malloc(size_t);
extern void free(void *);
extern void *calloc(size_t, size_t);
extern void *realloc(void *, size_t);
extern int mblen(const char *, size_t);
extern size_t mbstowcs(wchar_t *restrict, const char *restrict, size_t);
extern int mbtowc(wchar_t *restrict, const char *restrict, size_t);
extern size_t wcstombs(char *restrict, const wchar_t *restrict, size_t);
extern int wctomb(char *, wchar_t);

#endif
