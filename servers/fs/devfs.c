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
#include <stddef.h>
#include <string.h>
#include <set/hash.h>
#include "../../lib/libserv/libserv.h"
#include "devfs.h"
#include "api.h"
#include "fs.h"

static hash_t *hash;
static UB table[MAX_DEVICE][MAX_DEVICE_NAME];
static size_t num_device;

static unsigned int calc_hash(const void *key, const size_t size);
static int compare(const void *a, const void *b);


int device_init(void)
{
	num_device = 0;
	hash = hash_create(MAX_DEVICE, calc_hash, compare);

	return hash? TRUE:FALSE;
}

static unsigned int calc_hash(const void *key, const size_t size)
{
	unsigned char *p;
	int v = 0;

	for (p = (unsigned char*)key; *p; p++) {
		v <<= 8;
		v |= *p;
		v %= size;
	}

	return v;
}

static int compare(const void *a, const void *b)
{
	return strcmp(a, b);
}

W psc_bind_device_f(RDVNO rdvno, struct posix_request *req)
{
	UB *name = req->param.par_bind_device.name;
	ID port = req->param.par_bind_device.port;

	if (!hash || num_device >= MAX_DEVICE) {
		put_response(rdvno, ENOMEM, -1, 0);

		return FALSE;
	}

	dbg_printf("[FS] bind(%s, %d)\n", name, port);
	strcpy((void*)(table[num_device]), (void*)name);

	if (hash_put(hash, table[num_device], (void*)port)) {
		num_device++;
		put_response(rdvno, EOK, 0, 0);

		return TRUE;

	} else {
		put_response(rdvno, EPERM, -1, 0);

		return FALSE;
	}
}

ID device_find(const UB *name)
{
	return (ID)(hash_get(hash, name));
}
