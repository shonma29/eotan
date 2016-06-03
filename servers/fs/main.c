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
#include <boot/init.h>
#include <nerve/global.h>
#include <nerve/kcall.h>
#include "api.h"
#include "fs.h"
#include "devfs/devfs.h"
#include "../../kernel/mpu/mpufunc.h"

static fs_request request;
static void (*syscall[])(fs_request*) = {
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


static int initialize(void)
{
	if (!init_port()) {
		dbg_printf("fs: init_port failed\n");
		return -1;
	}

	if (!device_init())
		return -1;

	init_fs();
	init_process();

	if (device_find(sysinfo->root.device)
			&& device_find(get_device_id(DEVICE_MAJOR_CONS, 0))) {
		if (mount_root(sysinfo->root.device, sysinfo->root.fstype, 0)) {
			dbg_printf("fs: mount_root(%x, %d) failed\n",
					sysinfo->root.device,
					sysinfo->root.fstype);
		} else {
			dbg_printf("fs: mount_root(%x, %d) succeeded\n",
					sysinfo->root.device,
					sysinfo->root.fstype);
			exec_init(INIT_PID, INIT_PATH_NAME);
		}
	}

	dbg_printf("fs: start\n");

	return 0;
}

void start(VP_INT exinf)
{
	if (initialize()) {
		kcall->thread_end_and_destroy();
		return;
	}

	for (;;) {
		if (get_request(&(request.packet), &(request.rdvno)) < 0)
			continue;

		if (request.packet.operation >= sizeof(syscall) / sizeof(syscall[0]))
			error_response(request.rdvno, ENOTSUP);

		else
			syscall[request.packet.operation](&request);
	}
}
