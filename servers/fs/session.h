#ifndef _FS_SESSION_H_
#define _FS_SESSION_H_
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
#include "fs.h"

typedef struct _session_t {
	node_t node;
	//TODO add thread_id list
	struct permission permission;
	vnode_t *cwd;
	tree_t files;
} session_t;

extern void session_initialize(void);

extern session_t *session_create(const pid_t);
extern void session_destroy(session_t *);
extern session_t *session_find(const pid_t);

extern int session_create_desc(struct file **, session_t *, const int);
extern int session_destroy_desc(session_t *, const int);
extern struct file *session_find_desc(const session_t *, const int);

extern int session_get_path(char *, vnode_t **,
		const session_t *, const int, const char *);
extern int session_get_path2(char *, vnode_t **, const session_t *,
		const vnode_t *, const int, const char *);

#endif
