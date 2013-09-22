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
#include <kcall.h>
#include <string.h>
#include <boot/init.h>
#include <mpu/config.h>
#include <region.h>
#include <setting.h>
#include "fs.h"

typedef struct {
	char *arg0;
	char *arg1;
	char *env0;
	char buf[0];
} init_arg_t;

static void dummy(void);
static W create_init(ID process_id, ID thread_id);


W exec_init(ID process_id, char *pathname)
{
	struct posix_request req;
	T_CTSK pk_ctsk = {
		TA_HLNG,
		NULL,
		dummy,
		pri_user_foreground,
		USER_STACK_SIZE,
		NULL,
		process_id
	};
	W err;
	init_arg_t *p;
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	req.param.par_execve.stsize = sizeof(*p) * 3
			+ strlen(pathname) + 1;
	p = malloc(req.param.par_execve.stsize);
	if (!p)
		return ENOMEM;

	p->arg0 = p->buf;
	p->arg1 = NULL;
	p->env0 = NULL;
	strcpy(p->buf, pathname);
	req.param.par_execve.stackp = (VP)p;

	req.caller = kcall->thread_create_auto(&pk_ctsk);
	if (req.caller < 0) {
		free(p);
		return ESVC;
	}

	err = create_init(process_id, req.caller);
	if (!err)
		err = exec_program(&req, process_id, pathname);

	free(p);

	if (err) {
		//TODO destroy vmtree and process
		kcall->thread_destroy(req.caller);
	}

	kcall->thread_start(req.caller);

	dbg_printf("[MM] exec_init(%d, %s)\n", process_id, pathname);

	return err;
}

static void dummy(void)
{
}

static W create_init(ID process_id, ID thread_id)
{
	struct proc *p;
	W err;
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	dbg_printf("[MM] create_init(%d, %d)\n", process_id, thread_id);

	if ((process_id < INIT_PID)
			|| (process_id >= MAX_PROCESS))
		return EINVAL;

	err = proc_get_procp(process_id, &p);
	if (err)
		return err;

	if (p->proc_status == PS_DORMANT)
		return EINVAL;

	memset(p, 0, sizeof(*p));
/* if set explicitly
	p->proc_status = PS_DORMANT;
	p->proc_next = NULL;
*/
	p->proc_maintask = thread_id;
	p->proc_signal_handler = 0;
	p->proc_uid = INIT_UID;
	p->proc_gid = INIT_GID;
	p->proc_euid = INIT_EUID;
	p->proc_egid = INIT_EGID;
	p->proc_umask = INIT_UMASK;
	p->proc_status = PS_RUN;

	p->proc_workdir = rootfile;
	rootfile->i_refcount++;

	p->proc_pid = process_id;
	p->proc_ppid = INIT_PPID;

	err = open_special_dev(p);
	if (err) {
		dbg_printf("[MM] can't open special files\n");
		return err;
	}

	kcall->region_create(thread_id, TEXT_REGION, 0,
			0x7fffffff, 0x7fffffff, INIT_ACCESS);
	kcall->region_create(thread_id, DATA_REGION, 0,
			0x7fffffff, 0x7fffffff, INIT_ACCESS);
	kcall->region_create(thread_id, HEAP_REGION, 0,
			0, 0x7fffffff, INIT_ACCESS);
	err = create_vm_tree(p);
	if (err)
		return err;

	set_local(process_id, thread_id);

	return EOK;
}

