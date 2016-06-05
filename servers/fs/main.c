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
#include <major.h>
#include <services.h>
#include <boot/init.h>
#include <fs/nconfig.h>
#include <mpu/memory.h>
#include <nerve/global.h>
#include <nerve/kcall.h>
#include <set/slab.h>
#include "api.h"
#include "fs.h"
#include "devfs/devfs.h"
#include "../../kernel/mpu/mpufunc.h"

static slab_t request_slab;
static int (*syscall[])(fs_request*) = {
	if_chdir,
	if_chmod,
	if_close,
	if_dup,
	if_exec,
	if_exit,
	if_fork,
	if_fstat,
	if_link,
	if_lseek,
	if_mkdir,
	if_open,
	if_read,
	if_rmdir,
	if_unlink,
	if_waitpid,
	if_write,
	if_getdents,
	if_mount,
	if_statvfs,
	if_unmount,
	if_kill,
	if_dup2,
};

static int initialize(void);
static bool port_init(void);
static void request_init(void);


static int initialize(void)
{
	if (!port_init()) {
		dbg_printf("fs: init_port failed\n");
		return -1;
	}

	if (!device_init())
		return -1;

	request_init();
	fs_init();
	init_process();

	if (device_find(sysinfo->root.device)
			&& device_find(get_device_id(DEVICE_MAJOR_CONS, 0))) {
		if (fs_mount_root(sysinfo->root.device, sysinfo->root.fstype, 0)) {
			dbg_printf("fs: fs_mount_root(%x, %d) failed\n",
					sysinfo->root.device,
					sysinfo->root.fstype);
		} else {
			dbg_printf("fs: fs_mount_root(%x, %d) succeeded\n",
					sysinfo->root.device,
					sysinfo->root.fstype);
			exec_init(INIT_PID, INIT_PATH_NAME);
		}
	}

	dbg_printf("fs: start\n");

	return 0;
}

static bool port_init(void)
{
	struct t_cpor packet = {
		TA_TFIFO,
		sizeof(struct posix_request),
		sizeof(struct posix_response)
	};

	return kcall->port_create(PORT_FS, &packet) == E_OK;
}

static void request_init(void)
{
	request_slab.unit_size = sizeof(fs_request);
	request_slab.block_size = PAGE_SIZE;
	request_slab.min_block = 1;
	request_slab.max_block = MAX_REQUEST
			/ ((PAGE_SIZE - sizeof(slab_block_t))
					/ sizeof(fs_request));
	request_slab.palloc = kcall->palloc;
	request_slab.pfree = kcall->pfree;
	slab_create(&request_slab);
}

void start(VP_INT exinf)
{
	if (initialize()) {
		kcall->thread_end_and_destroy();
		return;
	}

	for (;;) {
		fs_request *req = slab_alloc(&request_slab);
		ER_UINT size;

		if (!req) {
			kcall->thread_sleep();
			continue;
		}

		size = kcall->port_accept(PORT_FS, &(req->rdvno),
				&(req->packet));
		if (size == sizeof(struct posix_request)) {
			int result;

			if (req->packet.operation >=
					sizeof(syscall) / sizeof(syscall[0]))
				result = ENOTSUP;

			else
				result = syscall[req->packet.operation](req);

			if (result > 0)
				put_response(req->rdvno, result, -1, 0);
		}

		else if (size < 0)
			dbg_printf("fs: acp_por failed %d\n", size);

		else
			put_response(req->rdvno, EINVAL, -1, 0);

		slab_free(&request_slab, req);
	}
}
