#ifndef __HMI_H__
#define __HMI_H__
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
#include <console.h>
#include <dev/device.h>
#include <fs/protocol.h>
#include <sys/syslimits.h>
#include <hmi/draw.h>
#include <hmi/window.h>
#include "terminal.h"

#define MYNAME "cons"

#define INTERRUPT_QUEUE_SIZE (1024)
#define REQUEST_QUEUE_SIZE (256)

typedef struct _hmi_interrupt_t {
	int type;
	int data;
} hmi_interrupt_t;

typedef struct {
	fsmsg_t packet;
	int tag;
} fs_request;

extern ER_ID accept_tid;
extern ID cons_mid;
extern volatile lfq_t req_queue;
extern volatile lfq_t unused_queue;

extern Screen screen0;
extern esc_state_t state0;

// reply.c
extern int reply(fs_request *, const size_t);
extern int reply_error(fs_request *, const int, const int, const int);

// window.c
extern Display *display;
extern esc_state_t state2;
extern esc_state_t state7;

extern int window_initialize(void);
extern int create_window(window_t **, const int, const int,
		const int, const int, const int, const char *,
		Screen *);
extern window_t *find_window(const int);
#if 0
extern int remove_window(const int);
#endif
// draw.c
extern ER_UINT draw_write(const UW, const char *);

// event.c
extern volatile lfq_t hmi_queue;
extern ER_UINT (*reader)(const int);

extern void hmi_handle(const int, const int);
extern ER_UINT consctl_write(const UW, const char *);
extern int event_initialize(void);

// file.c
extern void if_read(fs_request *);
extern void if_write(fs_request *);
extern void if_clunk(fs_request *);

#endif
