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
#include <unistd.h>
#include <mpu/_tunnel_registers.h>
#include "sys.h"

static pid_t call_fork(const uintptr_t, const uintptr_t, const uintptr_t,
		const uintptr_t, const uintptr_t, const int);
static int _fork_entry();


pid_t rfork(int flags)
{
	return _tunnel_registers(flags, call_fork);
}

static pid_t call_fork(const uintptr_t ebx, const uintptr_t ebp,
		const uintptr_t esi, const uintptr_t edi, const uintptr_t ecx,
		const int flags)
{
	sys_args_t args = {
		syscall_rfork,
		flags,
		((int) __builtin_frame_address(0)) + sizeof(uintptr_t) * 2,
		(int) _fork_entry
	};
	return _syscall(&args, sizeof(args));
}

static int _fork_entry(void)
{
	return _tunnel_out();
}
