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
#include <sys/errno.h>
#include <sys/syslimits.h>
#include "../../lib/libserv/libmm.h"
#include "../../lib/libserv/libserv.h"
#include "api.h"
#include "procfs/process.h"

#define STACK_TAIL (LOCAL_ADDR - PAGE_SIZE)
#define MAX_ENV (10)

typedef struct {
	int argc;
	char **argv;
	char **envp;
	char *arg0;
	char *arg1;
	char *env0;
	char *env1;
	char buf[0];
} init_arg_t;

//TODO where to define?
static char envpath[] = "PATH=/bin";
static char buf[sizeof(init_arg_t) + PATH_MAX + 1 + MAX_ENV];


W exec_init(ID process_id, char *pathname)
{
	//TODO add strnlen
//	size_t pathlen = strnlen(pathname, PATH_MAX + 1);
	size_t pathlen = strlen(pathname);
	if (pathlen > PATH_MAX)
		return ENAMETOOLONG;

	pathlen++;

	pm_args_t req;
	req.arg3 = (sizeof(init_arg_t) + pathlen
			+ strlen(envpath) + 1 + sizeof(int) - 1)
			& ~(sizeof(int) - 1);
	if (req.arg3 > sizeof(buf))
		return ENOMEM;

	if (process_create(process_id, 0, 0, 0) == -1)
		return ENOMEM;

	session_t *session = session_create(process_id);
	if (!session)
		return ENOMEM;

	session->permission.uid = INIT_UID;
	session->permission.gid = INIT_GID;
	session->cwd = rootfile;
	rootfile->refer_count++;

	size_t offset = STACK_TAIL - req.arg3;
	init_arg_t *p = (init_arg_t*)buf;
	p->argc = 1;
	p->argv = (char**)(offsetof(init_arg_t, arg0) + offset);
	p->envp = (char**)(offsetof(init_arg_t, env0) + offset);
	p->arg0 = (char*)(offsetof(init_arg_t, buf) + offset);
	p->arg1 = NULL;
	p->env0 = (char*)(sizeof(init_arg_t) + pathlen + offset);
	p->env1 = NULL;
	strcpy(p->buf, pathname);
	strcpy(&(buf[sizeof(init_arg_t) + pathlen]), envpath);
	req.arg2 = (W)p;

	log_info("fs: exec_init(%d, %s)\n", process_id, pathname);
	return exec_program(&req, session, session->cwd, pathname);
}
