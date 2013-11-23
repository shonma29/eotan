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
#include <local.h>
#include <mm.h>
#include <services.h>
#include <nerve/kcall.h>


int vunmap(ID id, VP start, UW size)
{
//	thread_local_t *local = _get_local();
	mm_args_t args;
	mm_reply_t *reply = (mm_reply_t*)&args;
	ER_UINT reply_size;
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	args.syscall_no = mm_syscall_vunmap;
	args.arg1 = (long int)id;
	args.arg2 = (long int)start;
	args.arg3 = (long int)size;
	reply_size = kcall->port_call(PORT_MM, &args, sizeof(args));

	if (reply_size == sizeof(*reply)) {
//		local->error_no = reply->error_no;
		return reply->result;

	} else {
//		local->error_no = ESVC;
		return -1;
	}
}
