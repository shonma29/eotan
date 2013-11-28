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
#include <errno.h>
#include <mm.h>
#include <services.h>
#include <nerve/kcall.h>
#include "../../lib/libserv/libserv.h"
#include "interface.h"
#include "process.h"

static kcall_t *kcall = (kcall_t*)KCALL_ADDR;

static int (*funcs[])(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args) = {
	mm_clock_gettime,
	mm_process_create,
	mm_process_destroy,
	mm_process_duplicate,
	mm_process_copy_stack,
	mm_process_set_context,
	mm_vmap,
	mm_vunmap,
	mm_vmstatus
};

#define BUFSIZ (sizeof(mm_args_t))
#define NUM_OF_FUNCS (sizeof(funcs) / sizeof(void*))
#define MYNAME "mm:"

static ER init(void)
{
	T_CPOR pk_cpor = { TA_TFIFO, BUFSIZ, BUFSIZ };

	process_initialize();

	return kcall->port_create(PORT_MM, &pk_cpor);
}

static void doit(void)
{
	for (;;) {
		mm_args_t args;
		mm_reply_t reply;
		RDVNO rdvno;
		int result;
		ER_UINT size = kcall->port_accept(PORT_MM, &rdvno, &args);

		if (size < 0) {
			dbg_printf(MYNAME "acp_por failed %d\n", size);
			break;
		}

		if (size != sizeof(mm_args_t)) {
			reply.result = -1;
			reply.error_no = EINVAL;
			result = reply_failure;

		} else if (args.syscall_no > NUM_OF_FUNCS) {
			reply.result = -1;
			reply.error_no = ENOSUP;
			result = reply_failure;

		} else
			result = funcs[args.syscall_no](&reply, rdvno, &args);

		switch (result) {
		case reply_success:
		case reply_failure:
			result = kcall->port_reply(rdvno, &reply,
					sizeof(reply));
			if (result != E_OK)
				dbg_printf(MYNAME "rpl_rdv failed %d\n",
						result);
			break;

		default:
			break;
		}
	}
}

void start(VP_INT exinf)
{
	ER error = init();

	if (error)
		dbg_printf(MYNAME "cre_por failed %d\n", error);

	else {
		dbg_printf(MYNAME "start\n");
		doit();
		dbg_printf(MYNAME "end\n");

		error = kcall->port_destroy(PORT_MM);
		if (error)
			dbg_printf(MYNAME "del_por failed %d\n", error);
	}

	kcall->thread_end_and_destroy();
}
