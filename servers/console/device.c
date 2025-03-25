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
#include <string.h>
#include <nerve/kcall.h>
#include "console.h"

typedef struct {
	char *name;
	driver_t driver;
} driver_table_t;

static int cons_write(struct file *, copier_t *, const off_t, const size_t,
		size_t *);
static int null_read(struct file *, copier_t *, const off_t, const size_t,
		size_t *);
static int null_write(struct file *, copier_t *, const off_t, const size_t,
		size_t *);
static int zero_read(struct file *, copier_t *, const off_t, const size_t,
		size_t *);

driver_table_t table[] = {
	{ "cons", { true, null_read, cons_write } },
	{ "null", { true, null_read, null_write } },
	{ "zero", { true, zero_read, null_write } }
};

static char zero_buf[1024];
static char cons_buf[1024];

#define CONS_MAX (sizeof(cons_buf) - 1)


int device_initialize(void)
{
	memset(zero_buf, 0, sizeof(zero_buf));
	cons_buf[CONS_MAX] = '\0';
	return 0;
}

driver_t *device_lookup(const char *name)
{
	for (int i = 0; i < sizeof(table) / sizeof(table[0]); i++)
		if (!strcmp(name, table[i].name))
			return &(table[i].driver);

	return NULL;
}

static int cons_write(struct file *file, copier_t *copier, const off_t offset,
		const size_t size, size_t *wrote_len)
{
	int result = 0;
	size_t rest = size;
	while (rest) {
		size_t len = (rest > CONS_MAX) ? CONS_MAX : rest;
		result = copier->copy(cons_buf, copier, len);
		if (result)
			break;

		kcall->printk("%s", cons_buf);
		rest -= len;
	}

	*wrote_len = size - rest;
	return 0;
}

static int null_read(struct file *file, copier_t *copier, const off_t offset,
		const size_t size, size_t *read_len)
{
	*read_len = 0;
	return 0;
}

static int null_write(struct file *file, copier_t *copier, const off_t offset,
		const size_t size, size_t *wrote_len)
{
	*wrote_len = size;
	return 0;
}

static int zero_read(struct file *file, copier_t *copier, const off_t offset,
		const size_t size, size_t *read_len)
{
	int result = 0;
	size_t rest = size;
	while (rest) {
		size_t len = (rest > sizeof(zero_buf)) ?
				sizeof(zero_buf) : rest;
		result = copier->copy(copier, zero_buf, len);
		if (result)
			break;

		rest -= len;
	}

	*read_len = size - rest;
	return result;
}
