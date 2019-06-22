#ifndef _PM_H_
#define _PM_H_
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

typedef enum {
	pm_syscall_fork = 0,
	pm_syscall_exec = 1,
	pm_syscall_chdir = 2,
	pm_syscall_create = 3,
	pm_syscall_remove = 4,
	pm_syscall_fstat = 5,
	pm_syscall_chmod = 6,
	pm_syscall_open = 7,
	pm_syscall_lseek = 8,
	pm_syscall_read = 9,
	pm_syscall_write = 10,
	pm_syscall_close = 11
} pm_syscall_e;

typedef struct {
	pm_syscall_e operation;
	int process_id;
	int arg1;
	int arg2;
	int arg3;
	int arg4;
	int arg5;
} pm_args_t;

typedef struct {
	int result1;
	int result2;
	int error_no;
} pm_reply_t;

#endif
