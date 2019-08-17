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
#include <services.h>
#include <string.h>
#include "pm.h"
#include "process.h"
#include "../../lib/libserv/libserv.h"

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


int exec_init(const pid_t process_id, char *pathname)
{
	size_t pathlen = strlen(pathname);
	if (pathlen > PATH_MAX)
		return ENAMETOOLONG;

	pathlen++;

	mm_args_t args;
	args.arg3 = (sizeof(init_arg_t) + pathlen
			+ strlen(envpath) + 1 + sizeof(int) - 1)
			& ~(sizeof(int) - 1);
	if (args.arg3 > sizeof(buf))
		return ENOMEM;

	int result = create_init(process_id);
	if (result)
		return result;

	size_t offset = STACK_TAIL - args.arg3;
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

	args.arg1 = (int)pathname;
	args.arg2 = (int)p;

	mm_process_t *process = get_process(process_id);
	result = attach(process, PORT_MM << 16);
	if (result) {
		//TODO destroy process
log_info("mm: init attach %d\n", result);
		return result;
	}

	mm_reply_t reply;
	result = process_exec(&reply, process, PORT_MM,
			&args);
	log_info("mm: exec_init(pid=%d, %s) r=%d e=%d\n",
			process_id, pathname, result, reply.data[0]);
	return (result ? reply.data[0] : 0);
}
