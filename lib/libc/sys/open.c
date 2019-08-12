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
#include <fcntl.h>
#include <mm.h>
#include <services.h>
#include <stdarg.h>


int open(const char *path, int oflag, ...)
{
	mm_args_t args;
	args.arg1 = (int)path;

	if (oflag & O_CREAT) {
		args.syscall_no = mm_syscall_create;
		args.arg2 = oflag & ~O_CREAT;

		va_list list;
		va_start(list, oflag);
		args.arg3 = va_arg(list, int);
//TODO 9p create sequence
	} else {
		args.syscall_no = mm_syscall_open;
		args.arg2 = oflag;
	}

	ER_UINT reply_size = cal_por(PORT_MM, 0xffffffff, &args, sizeof(args));
	mm_reply_t *reply = (mm_reply_t*)&args;
	if (reply_size == sizeof(*reply)) {
		if (reply->result == -1)
			_set_local_errno(reply->data[0]);

		return reply->result;
	} else {
		_set_local_errno(ECONNREFUSED);
		return (-1);
	}
}
