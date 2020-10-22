#ifndef _SYS_SYSCALL_H_
#define _SYS_SYSCALL_H_
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
	syscall_fork = 0,
	syscall_exec = 1,
	syscall_wait = 2,
	syscall_exit = 3,
	syscall_vmap = 4,
	syscall_vunmap = 5,
	syscall_sbrk = 6,
	syscall_chdir = 7,
	syscall_dup = 8,
	syscall_lseek = 9,
	syscall_attach = 10,
	syscall_open = 11,
	syscall_create = 12,
	syscall_read = 13,
	syscall_write = 14,
	syscall_close = 15,
	syscall_remove = 16,
	syscall_stat = 17,
	syscall_fstat = 18,
	syscall_chmod = 19,
	syscall_clock_gettime = 20,
	syscall_thread_find = 21,
	syscall_kill = 22
} syscall_e;

typedef struct {
	syscall_e syscall_no;
	int arg1;
	int arg2;
	int arg3;
	int arg4;
	int arg5;
} sys_args_t;

typedef struct {
	int result;
	int data[2];
} sys_reply_t;

#endif
