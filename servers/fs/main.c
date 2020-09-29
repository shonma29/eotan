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
#include <services.h>
#include <nerve/global.h>
#include <nerve/kcall.h>
#include <sys/errno.h>
#include "api.h"
#include "fs.h"
#include "session.h"
#include "../../lib/libserv/libserv.h"

struct fs_func {
	void (*call)(fs_request *);
	size_t max;
};

static struct fs_func func_table[] = {
	{ if_attach, MESSAGE_SIZE(Tattach) },
	{ if_walk, MESSAGE_SIZE(Twalk) },
	{ if_open, MESSAGE_SIZE(Topen) },
	{ if_create, MESSAGE_SIZE(Tcreate) },
	{ if_read, MESSAGE_SIZE(Tread) },
	{ if_write, MESSAGE_SIZE(Twrite) },
	{ if_clunk, MESSAGE_SIZE(Tclunk) },
	{ if_remove, MESSAGE_SIZE(Tremove) },
	{ if_stat, MESSAGE_SIZE(Tstat) },
	{ if_wstat, MESSAGE_SIZE(Twstat) }
};
#define NUM_OF_FUNC (sizeof(func_table) / sizeof(func_table[0]))

static fs_request req;

static int initialize(void);


static int initialize(void)
{
	session_initialize();

	int result = fs_initialize((int) (sysinfo->root.device),
			sysinfo->root.block_size);
	if (result) {
		log_err(MYNAME ": fs_initialize(%s, %d) failed %d\n",
				sysinfo->root.device, sysinfo->root.fstype,
				result);
		return -1;
	}

	log_info(MYNAME ": fs_mount(%s, %d)\n",
			sysinfo->root.device, sysinfo->root.fstype);

	struct t_cpor pk_cpor = {
		TA_TFIFO,
		sizeof(fsmsg_t),
		sizeof(fsmsg_t)
	};
	result = kcall->ipc_open(&pk_cpor);
	if (result) {
		log_err(MYNAME ": ipc_open failed %d\n", result);
		return -1;
	}

	return 0;
}

void start(VP_INT exinf)
{
	if (initialize()) {
		kcall->thread_end_and_destroy();
		return;
	}

	log_info(MYNAME ": start\n");

	for (;;) {
		ER_UINT size = kcall->ipc_receive(PORT_FS, &(req.tag),
				&(req.packet));
		if (size < 0) {
			log_err(MYNAME ": receive failed %d\n", size);
			continue;
		}

		if (size < sizeof(req.packet.header)) {
			//TODO what is tag?
			reply_error(req.tag, 0, 0, EPROTO);
			continue;
		}
/*
		//TODO validate session
		pid_t pid = thread_find(unpack_tid(&req));
		if (pid == -1) {
			log_err(MYNAME ": find failed %d\n", pid);
			//TODO what to do?
			reply2(req.tag, EINVAL, -1, 0);
			continue;
		}
*/

		int result;
		if (req.packet.header.ident != IDENT)
			result = EPROTO;
		else if (req.packet.header.type >= NUM_OF_FUNC)
			result = ENOTSUP;
		else {
			if (size == func_table[req.packet.header.type].max) {
				func_table[req.packet.header.type].call(&req);
				result = 0;
			} else
				result = EPROTO;
		}

		if (result)
			//TODO what is tag?
			reply_error(req.tag, req.packet.header.token, 0,
					result);
	}
}
