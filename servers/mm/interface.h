#ifndef _INTERFACE_H_
#define _INTERFACE_H_
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
#include <mm.h>

#define EOK (0)

typedef enum {
	reply_success = 0,
	reply_failure = 1,
	reply_wait = 2
} mm_reply_type_e;

extern int mm_fork(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args);
extern int mm_exec(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args);
extern int mm_wait(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args);
extern int mm_exit(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args);
extern int mm_vmap(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args);
extern int mm_vunmap(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args);
extern int mm_sbrk(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args);
extern int mm_chdir(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args);
extern int mm_dup(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args);
extern int mm_lseek(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args);
extern int mm_clock_gettime(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args);
extern int mm_thread_find(mm_reply_t *reply, RDVNO rdvno, mm_args_t *args);

#endif
