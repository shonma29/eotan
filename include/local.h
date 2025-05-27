#ifndef _LOCAL_H_
#define _LOCAL_H_
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
#include <limits.h>
#include <stddef.h>
#include <nerve/config.h>
#include <sys/types.h>

typedef struct {
	pid_t pid;
	pid_t ppid;
	uid_t uid;
	gid_t gid;
	size_t wd_len;
	char wd[PATH_MAX];
	char name[PATH_MAX];
} process_local_t;

typedef struct {
	int error_no;
} thread_local_t;

#define MAIN_THREAD_LOCAL_ADDR (USER_STACK_END_ADDR - sizeof(thread_local_t))

extern thread_local_t *_get_thread_local(void);

static inline process_local_t *_get_process_local(void)
{
	return ((process_local_t *) PROCESS_LOCAL_ADDR);
}

static inline void _set_local_errno(const int error_no)
{

	(_get_thread_local())->error_no = error_no;
}

#endif
