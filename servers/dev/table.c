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
#include <dev/drivers.h>
#include <nerve/kcall.h>
#include <set/hash.h>
#include "../../lib/libserv/libserv.h"
#include "table.h"

static hash_t *hash;
static device_info_t table[MAX_DEVICE];
static size_t num_device = 0;

void *malloc(size_t size);
void free(void *p);
static unsigned int calc_hash(const void *key, const size_t size);
static int compare(const void *a, const void *b);


void *malloc(size_t size)
{
	if (size > PAGE_SIZE)
		return NULL;

	return kcall->palloc();
}

void free(void *p)
{
	kcall->pfree(p);
}

bool dev_initialize(void)
{
	hash = hash_create(MAX_DEVICE, calc_hash, compare);
	if (!hash) {
		log_err("dev: cannot create hash\n");
		return false;
	}

	for (int i = 0; i < sizeof(drivers) / sizeof(drivers[0]); i++) {
		vdriver_t *p = drivers[i](sysinfo);
		if (!p)
			continue;

		for (const vdriver_unit_t **unit = p->units; *unit; unit++) {
			device_info_t *tbl = &(table[num_device]);
			strcpy(tbl->name, (*unit)->name);
			tbl->unit = (*unit)->unit;
			tbl->driver = p;

			if (hash_put(hash, tbl->name, (void *) tbl))
				log_err("dev: attach failure(%s, %p)\n",
						tbl->name, p);
			else {
				num_device++;
				log_info("dev: attach(%s, %p)\n", tbl->name, p);
			}
		}
	}

	return true;
}

static unsigned int calc_hash(const void *key, const size_t size)
{
	unsigned int v = 0;
	for (unsigned char *p = (unsigned char *) key; *p; p++)
		v = v * 241 + *p;

	return (v % size);
}

static int compare(const void *a, const void *b)
{
	device_info_t *x = (device_info_t *) a;
	device_info_t *y = (device_info_t *) b;
	return strcmp(x->name, y->name);
}

device_info_t *dev_find(const char *name)
{
	return (device_info_t *) (hash_get(hash, name));
}
