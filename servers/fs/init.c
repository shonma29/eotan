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
#include <core.h>
#include <local.h>
#include <stddef.h>
#include <string.h>
#include <boot/init.h>
#include <mm/segment.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include "../../lib/libserv/libmm.h"
#include "fs.h"

#define STACK_TAIL (LOCAL_ADDR - PAGE_SIZE)

typedef struct {
	int argc;
	char **argv;
	char **envp;
	char *arg0;
	char *arg1;
	char *env0;
	char buf[0];
} init_arg_t;

static char buf[sizeof(init_arg_t) + MAX_NAMELEN + 1];

static W create_init(ID process_id);


W exec_init(ID process_id, char *pathname)
{
	struct posix_request req;
	W err;
	init_arg_t *p;
	size_t offset;
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	req.param.par_execve.stsize = sizeof(init_arg_t)
			+ strlen(pathname) + 1;
	req.param.par_execve.stsize = (req.param.par_execve.stsize
			+ sizeof(int) - 1)
			& ~(sizeof(int) - 1);
	if (req.param.par_execve.stsize > sizeof(buf))
		return ENOMEM;

	offset = STACK_TAIL - req.param.par_execve.stsize;
	p = (init_arg_t*)buf;
	p->argc = 1;
	p->argv = (char**)(offsetof(init_arg_t, arg0) + offset);
	p->envp = (char**)(offsetof(init_arg_t, env0) + offset);
	p->arg0 = (char*)(offsetof(init_arg_t, buf) + offset);
	p->arg1 = NULL;
	p->env0 = NULL;
	strcpy(p->buf, pathname);
	req.param.par_execve.stackp = (VP)p;

	err = create_init(process_id);
	if (!err)
		err = exec_program(&req, process_id, pathname);

	if (err) {
		//TODO destroy vmtree and process
		kcall->thread_destroy(req.caller);
		return err;
	}

	dbg_printf("[MM] exec_init(%d, %s)\n", process_id, pathname);

	return err;
}

static W create_init(ID process_id)
{
	struct proc *p;
	W err;

	dbg_printf("[MM] create_init(%d)\n", process_id);

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
	p->proc_signal_handler = 0;
	p->proc_uid = INIT_UID;
	p->proc_gid = INIT_GID;
	p->proc_umask = INIT_UMASK;
	p->proc_status = PS_RUN;

	p->proc_workdir = rootfile;
	rootfile->i_refcount++;

	p->proc_pid = process_id;
	p->proc_ppid = INIT_PPID;

	process_create(process_id, 0, 0, 0);

	err = open_special_dev(p);
	if (err) {
		dbg_printf("[MM] can't open special files\n");
		return err;
	}

	return EOK;
}
