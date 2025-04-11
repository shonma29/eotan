#ifndef _HMI_SESSION_H_
#define _HMI_SESSION_H_
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
#include <hmi/terminal.h>
#include <hmi/window.h>
#include "api.h"
#include "device.h"

typedef struct {
	list_t readers;
	char *buf;
	size_t size;
	int read_position;
	int write_position;
	unsigned long position_mask;
} event_buf_t;

typedef struct _session_t {
	node_t node;
	list_t brothers;
	tree_t files;
	window_t *window;
	esc_state_t *state;
	int tid;
	channel_type_e type;
	event_buf_t event;
} session_t;

struct file {
	node_t node;
	list_t requests;
	session_t *f_session;
	uint_fast32_t f_flag;
	driver_t *f_driver;
};

#define getSessionPtr(p) ((uintptr_t) p - offsetof(session_t, node))

static inline session_t *getSessionFromList(const list_t *p)
{
	return ((session_t *) ((uintptr_t) p - offsetof(session_t, brothers)));
}

extern list_t session_list;
extern session_t *focused_session;

extern void session_bind_terminal(esc_state_t *, const window_t *);
extern int session_initialize(void);

extern session_t *session_find_by_request(const fs_request_t *);

extern int session_create_file(struct file **, session_t *, const int);
extern int session_destroy_file(session_t *, const int);
extern void session_destroy_all_files(void);
extern struct file *session_find_file(const session_t *, const int);

#endif
