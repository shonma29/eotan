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
#include <init.h>
#include <libc.h>
#include <local.h>
#include <major.h>
#include <services.h>
#include <string.h>
#include <mm/config.h>
#include <nerve/kcall.h>
#include <sys/syscall.h>
#include "mm.h"

#define STACK_TAIL USER_STACK_END_ADDR

typedef struct {
	int argc;
	char **argv;
	char **envp;
	char *arg0;
	char *arg1;
	char *env0;
	char buf[0];
} init_arg_t;

static char buf[sizeof(init_arg_t) + PATH_MAX + sizeof(thread_local_t)];


void load(void)
{
	do {
		sys_args_t args;
		args.syscall_no = syscall_bind;
		args.arg1 = (int) DEVICE_CLASS_SERVER;
		args.arg2 = (int) "/";
		args.arg3 = MREPL;
		if (kcall->ipc_call(PORT_MM, &args, sizeof(args))
				!= sizeof(sys_reply_t)) {
			log_info("init: call failed\n");
			break;
		} else {
			sys_reply_t *reply = (sys_reply_t *) &args;
			if (reply->result) {
				log_info("init: attach failed %d\n",
						reply->result);
				break;
			}
		}

		char *pathname = INIT_PATH_NAME;
		size_t pathlen = strlen(pathname);
		if (pathlen >= PATH_MAX) {
			log_err("init: ENAMETOOLONG\n");
			break;
		}

		pathlen++;

		args.arg3 = roundUp(sizeof(init_arg_t) + pathlen, sizeof(int))
				+ sizeof(thread_local_t);
		if (args.arg3 > sizeof(buf)) {
			log_err("init: ENOMEM\n");
			break;
		}

		size_t offset = STACK_TAIL - args.arg3;
		init_arg_t *p = (init_arg_t *) buf;
		p->argc = 1;
		p->argv = (char **) (offsetof(init_arg_t, arg0) + offset);
		p->envp = (char **) (offsetof(init_arg_t, env0) + offset);
		p->arg0 = (char *) (offsetof(init_arg_t, buf) + offset);
		p->arg1 = NULL;
		p->env0 = NULL;
		strcpy(p->buf, pathname);

		args.syscall_no = syscall_exec;
		args.arg1 = (int) pathname;
		args.arg2 = (int) p;
		log_info("init: exec(%s)\n", pathname);
		kcall->ipc_call(PORT_MM, &args, sizeof(args));
	} while (false);

	kcall->thread_end_and_destroy();
}
