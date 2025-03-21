#ifndef _PIPE_SESSION_H_
#define _PIPE_SESSION_H_
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
#include <set/tree.h>

typedef enum {
	UNOPENED = 0,
	OPENING = 1,
	CLOSED = 2
} pipe_status_e;

typedef struct _pipe_data {
	struct _pipe_data *partner;
	list_t readers;
	list_t writers;
	char *buf;
	size_t size;
	int read_position;
	int write_position;
	int refer_count;
	unsigned long position_mask;
} pipe_channel_t;

typedef struct _session_t {
	node_t node;
	tree_t files;
	pipe_channel_t pair[2];
} session_t;

struct file {
	node_t node;
	pipe_channel_t *f_channel;
	uint_fast32_t f_flag;
};

#define getSessionPtr(p) ((uintptr_t) p - offsetof(session_t, node))

extern int session_initialize(void);

extern session_t *session_find_by_request(const fs_request_t *);

extern int session_create_file(struct file **, session_t *, const int);
extern int session_destroy_file(session_t *, const int);
extern void session_destroy_all_files(void);
extern struct file *session_find_file(const session_t *, const int);

#endif
