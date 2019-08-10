#ifndef _MM_H_
#define _MM_H_
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

typedef struct {
	unsigned int syscall_no;
	int arg1;
	int arg2;
	int arg3;
	int arg4;
	int arg5;
} mm_args_t;

typedef struct {
	int result;
	int data[2];
} mm_reply_t;

typedef enum {
	mm_syscall_fork = 0,
	mm_syscall_exec = 1,
	mm_syscall_wait = 2,
	mm_syscall_exit = 3,
	mm_syscall_vmap = 4,
	mm_syscall_vunmap = 5,
	mm_syscall_sbrk = 6,
	mm_syscall_chdir = 7,
	mm_syscall_dup = 8,
	mm_syscall_lseek = 9,
	mm_syscall_clock_gettime = 10,
	mm_syscall_thread_find = 11
} mm_syscall_e;

#endif
