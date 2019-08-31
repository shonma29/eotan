#ifndef _MM_PROXY_H_
#define _MM_PROXY_H_
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
#include "process.h"

extern char pathbuf1[];
extern char pathbuf2[];

static inline int create_token(const int thread_id, const mm_session_t *session)
{
	return ((thread_id << 16) | session->node.key);
}

extern int _attach(mm_process_t *, const int);
extern int _walk(mm_file_t **, mm_process_t *, const int, const char *,
		fsmsg_t *);
extern int _open(const mm_file_t *, const int, const int, fsmsg_t *);
extern int _read(const mm_file_t *, const int, const off_t, const size_t,
		char *, fsmsg_t *);
extern int _clunk(mm_session_t *, mm_file_t *, const int, fsmsg_t *);
extern int _fstat(struct stat *, const mm_file_t *, const int, fsmsg_t *);

#endif