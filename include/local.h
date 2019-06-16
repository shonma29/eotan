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
#include <core/types.h>
#include <sys/types.h>

#define LOCAL_ADDR 0x7fffd000

typedef struct {
	ID thread_id;
	pid_t process_id;
	int error_no;
	uid_t user_id;
	gid_t group_id;
	pid_t parent_process_id;
	size_t wd_len;
	char wd[PATH_MAX];
} thread_local_t;

static inline thread_local_t *_get_local(void)
{
	return (thread_local_t*)LOCAL_ADDR;
}

static inline int _get_local_errno(void)
{
	thread_local_t *local = (thread_local_t*)LOCAL_ADDR;

	return local->error_no;
}

static inline void _set_local_errno(const int error_no)
{
	((thread_local_t*)LOCAL_ADDR)->error_no = error_no;
}
#endif
