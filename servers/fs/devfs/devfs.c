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
#include <device.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <nerve/kcall.h>
#include <set/hash.h>
#include "../../lib/libserv/libserv.h"
#include "devfs.h"
#include "api.h"

static hash_t *hash;
static device_info_t table[MAX_DEVICE];
static size_t num_device;

void *malloc(size_t size);
void free(void *p);
static unsigned int calc_hash(const void *key, const size_t size);
static int compare(const void *a, const void *b);
static int dummy_error();
static int dummy_ok();
//TODO use off_t
static int devfs_read(vnode_t *ip, void *buf, const int offset,
		const size_t len, size_t *rlength);
//TODO use off_t
static int devfs_write(vnode_t *ip, void *buf, const int offset,
		const size_t len, size_t *rlength);

vfs_operation_t devfs_fsops = {
    dummy_error,
    dummy_error,
    dummy_error,
    dummy_error,
    dummy_error,
    dummy_error,
    dummy_error,
    dummy_error,
    dummy_error,
    dummy_error,
    dummy_error,
    dummy_error,
    dummy_error,
    dummy_ok,
    devfs_read,
    devfs_write
};

static vdriver_t *(*drivers[])(int) = {
	/* cons */
	(vdriver_t *(*)(int))(0x80300000),
	/* ramdisk */
	(vdriver_t *(*)(int))(0x80310000)
};


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

int device_init(void)
{
	int i;

	num_device = 0;

	hash = hash_create(MAX_DEVICE, calc_hash, compare);
	if (!hash) {
		log_err("devfs: cannot create hash\n");
		return FALSE;
	}

	for (i = 0; i < sizeof(drivers) / sizeof(drivers[0]); i++) {
		vdriver_t *p = drivers[i]((int)NULL);

		if (!p)
			continue;

		table[num_device].id = p->id;
		strcpy((char*)(table[num_device].name), (char*)(p->name));
		table[num_device].size = p->size;
		table[num_device].driver = p;

		if (hash_put(hash, (void*)(p->id),
				(void*)&(table[num_device])))
			log_err("devfs: attach failure(%x, %s, %d, %x)\n",
					p->id, p->name, p->size, p);

		else {
			num_device++;
			log_info("devfs: attach success(%x, %s, %d, %x)\n",
					p->id, p->name, p->size, p);
		}
	}

	return TRUE;
}

static unsigned int calc_hash(const void *key, const size_t size)
{
	return (UW)key % size;
}

static int compare(const void *a, const void *b)
{
	UW x = (UW)a;
	UW y = (UW)b;

	return (x == y)? 0:((x < y)? (-1):1);
}

device_info_t *device_find(const UW devid)
{
	return (device_info_t*)(hash_get(hash, (void*)devid));
}

static int dummy_error()
{
	return ENOTSUP;
}

static int dummy_ok()
{
	return 0;
}
//TODO use off_t
static int devfs_read(vnode_t *ip, void *buf, const int offset,
		const size_t len, size_t *rlength)
{
	return read_device(ip->dev, buf, offset, len, rlength);
}
//TODO use off_t
static int devfs_write(vnode_t *ip, void *buf, const int offset,
		const size_t len, size_t *rlength)
{
	return write_device(ip->dev, buf, offset, len, rlength);
}
