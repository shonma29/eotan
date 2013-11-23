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
#include <boot/init.h>
#include <nerve/global.h>
#include <set/hash.h>
#include "../../lib/libserv/libserv.h"
#include "devfs.h"
#include "api.h"
#include "fs.h"

static system_info_t *sysinfo = (system_info_t*)SYSTEM_INFO_ADDR;
static hash_t *hash;
static device_info_t table[MAX_DEVICE];
static size_t num_device;

static unsigned int calc_hash(const void *key, const size_t size);
static int compare(const void *a, const void *b);


int device_init(void)
{
	num_device = 0;
	hash = hash_create(MAX_DEVICE, calc_hash, compare);

	if (!hash) {
		dbg_printf("[FS] cannot create hash\n");
		return FALSE;
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

W psc_bind_device_f(RDVNO rdvno, struct posix_request *req)
{
	UW id = req->param.par_bind_device.id;
	UB *name = req->param.par_bind_device.name;
	ID port = req->param.par_bind_device.port;

	if (!hash || num_device >= MAX_DEVICE) {
		put_response(rdvno, ENOMEM, -1, 0);

		return FALSE;
	}

	dbg_printf("[FS] bind(%x, %s, %d)\n", id, name, port);
	table[num_device].id = id;
	strcpy((char*)(table[num_device].name), (char*)name);
	table[num_device].port = port;

	if (hash_put(hash, (void*)id, (void*)&(table[num_device]))) {
		put_response(rdvno, EPERM, -1, 0);

		return FALSE;

	} else {
		num_device++;
		put_response(rdvno, EOK, 0, 0);

		if (id == sysinfo->root.device) {
			if (mount_root(id, sysinfo->root.fstype, 0)) {
				dbg_printf("[FS] mount_root(%x, %d) failed\n",
						id, sysinfo->root.fstype);
			} else {
				dbg_printf("[FS] mount_root(%x, %d) succeeded\n",
						id, sysinfo->root.fstype);
				exec_init(INIT_PID, INIT_PATH_NAME);
			}
		}

		return TRUE;
	}
}

device_info_t *device_find(const UW devid)
{
	return (device_info_t*)(hash_get(hash, (void*)devid));
}
